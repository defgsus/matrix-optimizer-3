/** @file group.cpp

    @brief Group for all kinds of objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2014</p>
*/

#include "Group.h"
#include "io/DataStream.h"

namespace MO {

MO_REGISTER_OBJECT(Group)

Group::Group() :
    Object()
{
    setName("Group");
    setNumberOutputs(ST_TRANSFORMATION, 1);
}

Group::~Group() { }

void Group::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("group", 1);
}

void Group::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("group", 1);
}


} // namespace MO
