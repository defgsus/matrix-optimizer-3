/** @file track.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#include "track.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(Track)

Track::Track(QObject *parent) :
    Object(parent)
{
    setName("Track");
}

void Track::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("Track", 1);
}

void Track::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("Track", 1);
}


} // namespace MO
