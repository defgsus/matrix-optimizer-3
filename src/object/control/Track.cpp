/** @file track.cpp

    @brief Abstract track object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#include "Track.h"
#include "io/DataStream.h"


namespace MO {

Track::Track() :
    Object()
{
}

Track::~Track() { }

void Track::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("track", 1);
}

void Track::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("track", 1);
}



} // namespace MO
