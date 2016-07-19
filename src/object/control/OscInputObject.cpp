/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#include <QVector>
#include <QDebug>
#include <QByteArray>

#include "OscInputObject.h"
#include "object/Scene.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterText.h"
#include "tool/ValueSmoother.h"
#include "network/OscInput.h"
#include "network/OscInputs.h"
#include "io/DataStream.h"
#include "io/CurrentTime.h"

namespace MO {

MO_REGISTER_OBJECT(OscInputObject)

struct OscInputObject::Private
{
    struct Value
    {
        Value() : p_id(0) { }

        ParameterText * p_id;
        std::vector<ValueSmoother<Double>> smooth;
    };

    Private(OscInputObject * p)
        : p     (p)
        , receiver(new OscInputObjectReceiver(p))
        , osc   (0)
    {
        values.resize(10);
    }

    ~Private()
    {
        if (osc)
            osc->releaseRef("OscInputObject destroy");
        delete receiver;
    }

    void setParamVis();
    void setNumChan();
    void setPort();
    void getValue(const QString& id);
    void updateValueMap();

    OscInputObject * p;
    OscInputObjectReceiver* receiver;
    ParameterSelect * p_interpol;
    ParameterInt * p_numChan, * p_port;
    ParameterFloat * p_smoothTime;
    QVector<Value> values;
    QMap<QString, Value*> valueMap;

    OscInput * osc;
};

void OscInputObjectReceiver::onValueChanged(const QString &id)
{
    obj->onValueChanged(id);
}

OscInputObject::OscInputObject()
    : Object    ()
    , p_        (new Private(this))
{
    setName("OscInput");
}

OscInputObject::~OscInputObject()
{
    delete p_;
}

void OscInputObject::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("osci", 1);
}

void OscInputObject::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("osci", 1);
}

void OscInputObject::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("osc", tr("OSC"));

        p_->p_port = params()->createIntParameter(
                    "port", tr("udp port"),
                    tr("The port number"),
                    51000, 1, 65535, 1, true, false);

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
                    tr("The time it takes to follow the input value, in seconds"),
                    0.1, 0.01);
        p_->p_smoothTime->setMinValue(0.);

        p_->p_numChan = params()->createIntParameter(
                    "num_channel", tr("number channels"),
                    tr("The number of different channels in this object"),
                    1, 1, p_->values.size(), 1, true, false);

        for (int i=0; i<p_->values.size(); ++i)
        {
            p_->values[i].p_id = params()->createTextParameter(
                             QString("id_%1").arg(i),
                             tr("id %1").arg(i),
                             tr("The name of the osc value"),
                             TT_PLAIN_TEXT,
                             QString("/1/fader%1").arg(i),
                             true, false);
        }

    params()->endParameterGroup();
}

void OscInputObject::onParametersLoaded()
{
    Object::onParametersLoaded();
    p_->setNumChan();
    p_->setPort();
    p_->updateValueMap();
}

void OscInputObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
    if (p == p_->p_numChan)
        p_->setNumChan();
    if (p == p_->p_port)
        p_->setPort();

    for (const auto & v : p_->values)
    if (p == v.p_id)
    {
        p_->updateValueMap();
        emitConnectionsChanged();
    }
}

void OscInputObject::updateParameterVisibility()
{
    Object::updateParameterVisibility();
    p_->p_smoothTime->setVisible(
                p_->p_interpol->baseValue() != MATH::IT_NONE);
    p_->setParamVis();
}

QString OscInputObject::getOutputName(SignalType st, uint channel) const
{
    if (st != ST_FLOAT)
        return Object::getOutputName(st, channel);

    if ((int)channel >= p_->values.size())
        return QString("id %").arg(channel);
    else
        return p_->values[channel].p_id->baseValue();
}

void OscInputObject::setNumberThreads(uint num)
{
    Object::setNumberThreads(num);
    for (auto & v : p_->values)
    {
        v.smooth.resize(num);
    }
}

void OscInputObject::onValueChanged(const QString &id)
{
    p_->getValue(id);
}

void OscInputObject::Private::setParamVis()
{
    const int num = p_numChan->baseValue();
    for (int i=0; i<values.size(); ++i)
    {
        values[i].p_id->setVisible(i < num);
    }
}

void OscInputObject::Private::setNumChan()
{
    const int num = p_numChan->baseValue();
    p->setNumberOutputs(ST_FLOAT, num);
}

void OscInputObject::Private::setPort()
{
    if (osc)
        OscInputs::releaseListener(osc->port());
    const int num = p_port->baseValue();
    osc = OscInputs::getListener(num);

    QObject::connect(
                osc, SIGNAL(valueChanged(QString)),
                receiver, SLOT(onValueChanged(QString)),
                Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
}


void OscInputObject::Private::updateValueMap()
{
    valueMap.clear();
    for (Value & v : values)
        valueMap.insert(v.p_id->baseValue(), &v);
}

void OscInputObject::Private::getValue(const QString& id)
{
    auto i = valueMap.find(id);
    if (i == valueMap.end())
        return;

    //auto scene = p->sceneObject();
    //const Double ti = scene ? scene->sceneTime() : 0.;
    const Double
              ti = CurrentTime::time()
            , val = osc->value(id).toDouble();

    Value * v = i.value();
    for (auto & sm : v->smooth)
        sm.set(ti, val);
}

Double OscInputObject::valueFloat(uint chan, const RenderTime& time) const
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
