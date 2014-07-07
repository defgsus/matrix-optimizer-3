/** @file transformation.cpp

    @brief abstract object transformation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "transformation.h"
#include "io/datastream.h"


namespace MO {

Transformation::Transformation(QObject *parent) :
    Object(parent)
{
    setName("Transformation");
}

void Transformation::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("transf", 1);
}

void Transformation::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("transf", 1);
}

} // namespace MO

