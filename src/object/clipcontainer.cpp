/** @file clipcontainer.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include "clipcontainer.h"
#include "io/datastream.h"
#include "io/error.h"
#include "clip.h"

namespace MO {

MO_REGISTER_OBJECT(ClipContainer)

ClipContainer::ClipContainer(QObject *parent) :
    Object          (parent)
{
    setName("ClipContainer");
}

void ClipContainer::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("clipcon", 1);

}

void ClipContainer::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("clipcon", 1);
}

void ClipContainer::createParameters()
{
    Object::createParameters();
}


void ClipContainer::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}


} // namespace MO
