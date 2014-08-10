/** @file modulatorobjectfloat.cpp

    @brief A float sending modulator object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#include "modulatorobjectfloat.h"
#include "io/datastream.h"
#include "io/log.h"
#include "sequencefloat.h"

namespace MO {

MO_REGISTER_OBJECT(ModulatorObjectFloat)

ModulatorObjectFloat::ModulatorObjectFloat(QObject *parent) :
    ModulatorObject(parent)
{
    setName("ModulatorFloat");
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

Double ModulatorObjectFloat::value(Double time, uint thread) const
{
    return 0.0;
}

} // namespace MO
