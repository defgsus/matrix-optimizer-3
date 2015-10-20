/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#include <QVector>
#include <QDebug>
#include <QByteArray>

#include "oscinputobject.h"
#include "object/scene.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "tool/linearizerfloat.h"
#include "network/oscinput.h"
#include "network/oscinputs.h"
#include "io/datastream.h"
#include "io/currenttime.h"

namespace MO {

MO_REGISTER_OBJECT(OscInputObject)

struct OscInputObject::Private
{
    struct Value
    {
        Value() : linear(0), valueNew(0.), timeIn(0.) { }
        ~Value() { delete linear; }

        ParameterText * p_id;
        Double valueNew, timeIn;
        std::vector<Double> value, valueOut;
        LinearizerFloat * linear;
    };

    Private(OscInputObject * p)
        : p     (p)
        , osc   (0)
    {
        values.resize(10);
        for (auto & v : values)
            v.linear = new LinearizerFloat;
    }

    ~Private()
    {
        if (osc)
            osc->releaseRef();
    }

    void setParamVis();
    void setNumChan();
    void setPort();
    void getValue(const QString& id);
    void updateValueMap();
    void updateInterpolationMode();

    OscInputObject * p;
    ParameterSelect * p_interpol;
    ParameterInt * p_numChan, * p_port;
    ParameterFloat * p_smoothTime;
    QVector<Value> values;
    QMap<QString, Value*> valueMap;

    OscInput * osc;
};


OscInputObject::OscInputObject(QObject *parent)
    : Object    (parent)
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
            { LinearizerFloat::IM_NONE, LinearizerFloat::IM_LINEAR,
              LinearizerFloat::IM_SMOOTH, LinearizerFloat::IM_SMOOTH2 },
            LinearizerFloat::IM_NONE,
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
                             QString("value_%1").arg(i),
                             true, false);
        }

    params()->endParameterGroup();
}

void OscInputObject::onParametersLoaded()
{
    Object::onParametersLoaded();
    p_->updateInterpolationMode();
    p_->setNumChan();
    p_->setPort();
    p_->updateValueMap();
}

void OscInputObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
    if (p == p_->p_interpol)
        p_->updateInterpolationMode();
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
                p_->p_interpol->baseValue() != LinearizerFloat::IM_NONE);
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
        v.value.resize(num);
        v.valueOut.resize(num);
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
    connect(osc, SIGNAL(valueChanged(QString)),
            p, SLOT(onValueChanged(QString)),
            Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
}

void OscInputObject::Private::updateInterpolationMode()
{
    for (auto & v : values)
        v.linear->setInterpolationMode(
            (LinearizerFloat::InterpolationMode)p_interpol->baseValue());
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
    const Double ti = CurrentTime::time();

    Value * v = i.value();
    //v->value = osc->value(id).toDouble();
    for (uint i=0; i<v->value.size(); ++i)
        v->value[i] = v->valueOut[i];
    v->valueNew = osc->value(id).toDouble();
    v->timeIn = ti;
    //v->linear->insertValue(ti, v->value);
}

Double OscInputObject::valueFloat(uint chan, Double time, uint thread) const
{
    if ((int)chan >= p_->values.size())
        return 0.;

    if (p_->p_interpol->baseValue() == LinearizerFloat::IM_NONE)
        return p_->values[chan].valueNew;

    Private::Value & v = p_->values[chan];
    const Double st = std::max(0.0001, p_->p_smoothTime->value(time, thread));

    //if (thread == MO_AUDIO_THREAD)
    //    qInfo() << v.timeIn << st << time;
    if (time < v.timeIn)
        return v.value[thread];
    if (time >= v.timeIn + st)
    {
        return v.valueOut[thread] = v.value[thread] = v.valueNew;
    }
    else
    {
        Double t = (time - v.timeIn) / st;
        return v.valueOut[thread] = v.value[thread] + t * (v.valueNew - v.value[thread]);
    }

    //return p_->values[chan].linear->getValue(time);
}


} // namespace MO
