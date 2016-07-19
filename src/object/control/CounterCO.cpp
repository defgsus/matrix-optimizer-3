/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 4/27/2016</p>
*/

#if 0

#include "counterco.h"
#include "object/scene.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "tool/valuesmoother.h"
#include "io/datastream.h"
#include "io/currenttime.h"
#include "audio/tool/floatgate.h"

namespace MO {

MO_REGISTER_OBJECT(CounterCO)

struct CounterCO::Private
{
    struct Value
    {
        Value() : count(0), p_input(0) { }

        int count;
        ParameterFloat * p_input;
        AUDIO::FloatGate gate;
        std::vector<ValueSmoother<Double>> smooth;
    };

    Private(CounterCO * p)
        : p     (p)
    {
        values.resize(10);
    }

    ~Private()
    {
        if (osc)
            osc->releaseRef("CounterCO destroy");
        delete receiver;
    }

    void setParamVis();
    void setNumChan();
    void setPort();
    void getValue(const QString& id);

    CounterCO * p;
    ParameterSelect * p_interpol;
    ParameterInt * p_numChan;
    ParameterFloat * p_smoothTime;
    QVector<Value> values;
};

CounterCO::CounterCO()
    : Object    ()
    , p_        (new Private(this))
{
    setName("Counter");
}

CounterCO::~CounterCO()
{
    delete p_;
}

void CounterCO::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("count", 1);
}

void CounterCO::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("count", 1);
}

void CounterCO::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("counter", tr("counter"));
    initParameterGroupExpanded("counter");

        p_->p_interpol = params()->createSelectParameter(
            "interpolation", tr("interpolation"),
            tr("Selects the interpolation mode for smoothing input values"),
            { "none", "linear", "smooth", "smooth2" },
            { tr("none"), tr("linear"), tr("smooth"), tr("smooth steeper") },
            { tr("No interpolation"),
              tr("Linear interpolation"),
              tr("Smooth transition from one value to the next"),
              tr("Steep smooth transition from one value to the next") },
            { MATH::IT_NONE, MATH::IT_LINEAR,
              MATH::IT_SMOOTH, MATH::IT_SMOOTH2 },
              MATH::IT_NONE,
            true, false);

        p_->p_smoothTime = params()->createFloatParameter(
                    "smooth_time", tr("smoothing time"),
                    tr("The time it takes to follow the counted value, in seconds"),
                    0.1, 0.01);
        p_->p_smoothTime->setMinValue(0.);

        p_->p_numChan = params()->createIntParameter(
                    "num_channel", tr("number channels"),
                    tr("The number of different channels in this object"),
                    1, 1, p_->values.size(), 1, true, false);

        for (int i=0; i<p_->values.size(); ++i)
        {
            p_->values[i].p_input = params()->createFloatParameter(
                             QString("input_%1").arg(i),
                             tr("input %1").arg(i),
                             tr("Input signal that gets counted for each positive "
                                "crossing"),
                             0., true, true);
        }

    params()->endParameterGroup();
}

void CounterCO::onParametersLoaded()
{
    Object::onParametersLoaded();
    p_->setNumChan();
    p_->updateValueMap();
}

void CounterCO::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
    if (p == p_->p_numChan)
        p_->setNumChan();
}

void CounterCO::updateParameterVisibility()
{
    Object::updateParameterVisibility();
    p_->p_smoothTime->setVisible(
                p_->p_interpol->baseValue() != MATH::IT_NONE);
    p_->setParamVis();
}

QString CounterCO::getOutputName(SignalType st, uint channel) const
{
    if (st != ST_FLOAT)
        return Object::getOutputName(st, channel);

    if ((int)channel >= p_->values.size())
        return QString("count %").arg(channel);
    else
        return p_->values[channel].p_id->baseValue();
}

void CounterCO::setNumberThreads(uint num)
{
    Object::setNumberThreads(num);
    for (auto & v : p_->values)
    {
        v.smooth.resize(num);
    }
}

void CounterCO::Private::setParamVis()
{
    const int num = p_numChan->baseValue();
    for (int i=0; i<values.size(); ++i)
    {
        values[i].p_input->setVisible(i < num);
    }
}

void CounterCO::Private::setNumChan()
{
    const int num = p_numChan->baseValue();
    p->setNumberOutputs(ST_FLOAT, num);
}


void CounterCO::Private::getValue(const QString& id)
{
    auto i = valueMap.find(id);
    if (i == valueMap.end())
        return;

    const Double
              ti = CurrentTime::time()
            , val = osc->value(id).toDouble();

    Value * v = i.value();
    for (auto & sm : v->smooth)
        sm.set(ti, val);
}

Double CounterCO::valueFloat(uint chan, const RenderTime& time) const
{
    if ((int)chan >= p_->values.size())
        return 0.;

    if (p_->p_interpol->baseValue() == MATH::IT_NONE)
        return p_->values[chan].smooth[time.thread()].get();

    Private::Value & v = p_->values[chan];
    const Double st = std::max(0.0001, p_->p_smoothTime->value(time));

    return v.smooth[time.thread()].get(time.second(), st,
                                (MATH::InterpolationType)p_->p_interpol->baseValue());
}


} // namespace MO

#endif
