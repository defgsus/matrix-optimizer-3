/** @file dummy.cpp

    @brief Dummy object for skipping unknown objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include "dummy.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(Dummy)

Dummy::Dummy(QObject *parent) :
    Object(parent)
{
    setName("Dummy");
}

void Dummy::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("dummy", 1);
}

void Dummy::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("dummy", 1);
}


} // namespace MO
