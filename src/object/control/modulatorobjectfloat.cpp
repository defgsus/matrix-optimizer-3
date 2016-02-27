/** @file modulatorobjectfloat.cpp

    @brief A float sending modulator object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#include "modulatorobjectfloat.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(ModulatorObjectFloat)

ModulatorObjectFloat::ModulatorObjectFloat()
    : ModulatorObject   ()
    , p_value_          (0)
    , timeStamp_        (0.0)
    , offset_           (0.0)
{
    setName("ModulatorFloat");
    setNumberOutputs(ST_FLOAT, 1);
}

void ModulatorObjectFloat::serialize(IO::DataStream & io) const
{
    ModulatorObject::serialize(io);
    io.writeHeader("modobjf", 1);
}

void ModulatorObjectFloat::deserialize(IO::DataStream & io)
{
    ModulatorObject::deserialize(io);
    io.readHeader("modobjf", 1);
}

void ModulatorObjectFloat::createParameters()
{
    ModulatorObject::createParameters();

    params()->beginParameterGroup("modval", tr("value"));
    initParameterGroupExpanded("modval");

        p_value_ = params()->createFloatParameter("val", tr("value"),
                                       tr("A float value - sent to all receivers of the modulator"),
                                       0.0, 1.0);

        p_amp_ = params()->createFloatParameter("amp", tr("amplitude"),
                                       tr("Output multiplier"),
                                       1.0, 0.1);

    params()->endParameterGroup();
}

Double ModulatorObjectFloat::valueFloat(uint, const RenderTime& time) const
{
    return offset_ + p_value_->value(time)
                     * p_amp_->value(time);
}

void ModulatorObjectFloat::setValue(Double timeStamp, Double value)
{
    timeStamp_ = timeStamp;
    offset_ = value;
}

/*
Double ModulatorObjectFloat::getOffset_(Double time)
{
    Double fadeTime = 1.0 / 100.0;
}
*/

} // namespace MO
