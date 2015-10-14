/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#include "oscinputobject.h"
#include "object/param/parameters.h"
#include "object/param/parameterselect.h"
#include "tool/linearizerfloat.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(OscInputObject)

struct OscInputObject::Private
{
    Private(OscInputObject * p)
        : p(p)
    {

    }

    void updateInterpolationMode();

    OscInputObject * p;
    ParameterSelect * p_interpol;
    LinearizerFloat linear;
};


OscInputObject::OscInputObject(QObject *parent)
    : Object    (parent)
    , p_        (new Private(this))
{
    setName("OscInput");
    setNumberOutputs(ST_FLOAT, 1);

    p_->linear.insertValue(0, 0);
    p_->linear.insertValue(1, 1);
    p_->linear.insertValue(2, -2);
    p_->linear.insertValue(3, 3);
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

    params()->endParameterGroup();
}

void OscInputObject::onParametersLoaded()
{
    Object::onParametersLoaded();
    p_->updateInterpolationMode();
}

void OscInputObject::onParameterChanged(Parameter *p)
{
    Object::onParameterChanged(p);
    if (p == p_->p_interpol)
        p_->updateInterpolationMode();
}

void OscInputObject::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}

void OscInputObject::Private::updateInterpolationMode()
{
    linear.setInterpolationMode(
                (LinearizerFloat::InterpolationMode)p_interpol->baseValue());
}

Double OscInputObject::valueFloat(uint , Double time, uint ) const
{
    return p_->linear.getValue(time);
}


} // namespace MO
