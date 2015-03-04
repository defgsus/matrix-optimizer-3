/** @file modulatorobject.cpp

    @brief An abstract object that represents a modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#include "modulatorobject.h"
#include "io/datastream.h"


namespace MO {

ModulatorObject::ModulatorObject(QObject *parent) :
    Object(parent)
{
}

void ModulatorObject::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("modobj", 2);

    // v2
    io << p_uiId_;
}

void ModulatorObject::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    const int ver = io.readHeader("modobj", 2);

    if (ver >= 2)
        io >> p_uiId_;
}



} // namespace MO
