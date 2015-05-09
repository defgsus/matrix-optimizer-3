/** @file textobject.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.05.2015</p>
*/

#include "textobject.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(TextObject)

TextObject::TextObject(QObject *parent) :
    Object(parent)
{
    setName("Text");
}

void TextObject::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("text", 1);
}

void TextObject::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("text", 1);
}


} // namespace MO
