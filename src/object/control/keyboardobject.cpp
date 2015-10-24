/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#include <QVector>

#include "keyboardobject.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "tool/valuesmoother.h"
#include "tool/keyboardstate.h"
#include "tool/enumnames.h"
#include "io/datastream.h"
#include "io/currenttime.h"

namespace MO {

MO_REGISTER_OBJECT(KeyboardObject)

struct KeyboardObject::Private
{
    struct Value
    {
        Value() : keyCode(0), p_key(0) { }

        int keyCode;
        ParameterInt * p_key;
        std::vector<ValueSmoother<Double>> smooth;
    };

    Private(KeyboardObject * p)
        : p     (p)
    {
        values.resize(10);
    }

    void setParamVis();
    void setNumChan();
    void getKeyCodes();
    //void getValue(const QString& id);

    KeyboardObject * p;
    ParameterInt * p_numChan;
    //ParameterSelect * p_interpol;
    ParameterFloat
            * p_onValue, * p_offValue;//* p_smoothTime;
    QVector<Value> values;
};


KeyboardObject::KeyboardObject(QObject *parent)
    : Object    (parent)
    , p_        (new Private(this))
{
    setName("Keyboard");
}

KeyboardObject::~KeyboardObject()
{
    delete p_;
}

void KeyboardObject::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("key", 1);
}

void KeyboardObject::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("key", 1);
}

void KeyboardObject::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("key", tr("keyboard input"));
        /*
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
        */
        p_->p_numChan = params()->createIntParameter(
                    "num_channel", tr("number channels"),
                    tr("The number of different channels in this object"),
                    1, 1, p_->values.size(), 1, true, false);

        p_->p_onValue = params()->createFloatParameter(
                    "on_value", tr("on value"),
                    tr("The value that is output for a pressed key"),
                    1., true, true);
        p_->p_offValue = params()->createFloatParameter(
                    "off_value", tr("off value"),
                    tr("The value that is output for an unpressed key"),
                    0., true, true);

        for (int i=0; i<p_->values.size(); ++i)
        {
            p_->values[i].p_key = params()->createIntParameter(
                             QString("keycode_%1").arg(i),
                             tr("key %1").arg(i),
                             tr("The recognized key"),
                             Qt::Key_A + i, true, false);
            p_->values[i].p_key->setSpecificFlag(Parameter::SF_KEYCODE);
        }

    params()->endParameterGroup();
}

void KeyboardObject::onParametersLoaded()
{
    Object::onParametersLoaded();
    p_->setNumChan();
    p_->getKeyCodes();
}

void KeyboardObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
    if (p == p_->p_numChan)
        p_->setNumChan();

    p_->getKeyCodes();

    for (const auto & v : p_->values)
    if (p == v.p_key)
    {
        emitConnectionsChanged();
    }
}

void KeyboardObject::updateParameterVisibility()
{
    Object::updateParameterVisibility();
    /*p_->p_smoothTime->setVisible(
                p_->p_interpol->baseValue() != MATH::IT_NONE);*/
    p_->setParamVis();
}

QString KeyboardObject::getOutputName(SignalType st, uint channel) const
{
    if (st != ST_FLOAT)
        return Object::getOutputName(st, channel);

    if ((int)channel >= p_->values.size())
        return QString("key %").arg(channel);
    else
        return enumName( Qt::Key(p_->values[channel].p_key->baseValue()) );
}

void KeyboardObject::setNumberThreads(uint num)
{
    Object::setNumberThreads(num);
    for (auto & v : p_->values)
    {
        v.smooth.resize(num);
    }
}

void KeyboardObject::Private::setParamVis()
{
    const int num = p_numChan->baseValue();
    for (int i=0; i<values.size(); ++i)
    {
        values[i].p_key->setVisible(i < num);
    }
}

void KeyboardObject::Private::setNumChan()
{
    const int num = p_numChan->baseValue();
    p->setNumberOutputs(ST_FLOAT, num);
}

void KeyboardObject::Private::getKeyCodes()
{
    for (auto & v : values)
    {
        v.keyCode = v.p_key->baseValue();
    }
}

/*
void KeyboardObject::Private::getValue(const QString& id)
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
*/

Double KeyboardObject::valueFloat(uint chan, const RenderTime& time) const
{
    if ((int)chan >= p_->values.size())
        return 0.;

    //if (p_->p_interpol->baseValue() == MATH::IT_NONE)
    return KeyboardState::globalInstance().isDown(
            p_->values[chan].keyCode)
            ? p_->p_onValue->value(time)
            : p_->p_offValue->value(time);
    /*
    Private::Value & v = p_->values[chan];
    const Double st = std::max(0.0001, p_->p_smoothTime->value(time));

    return v.smooth[thread].get(time, st, (MATH::InterpolationType)p_->p_interpol->baseValue());
    */
}


} // namespace MO
