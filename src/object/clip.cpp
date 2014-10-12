/** @file clip.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include "clip.h"
#include "io/datastream.h"
#include "io/error.h"
#include "sequence.h"
#include "clipcontainer.h"

namespace MO {

MO_REGISTER_OBJECT(Clip)

Clip::Clip(QObject *parent) :
    Object          (parent)
{
    setName("Clip");
}

void Clip::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("clip", 1);

}

void Clip::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("clip", 1);
}

ClipContainer * Clip::clipContainer() const
{
    return qobject_cast<ClipContainer*>(parentObject());
}


void Clip::createParameters()
{
    Object::createParameters();
}


void Clip::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}


void Clip::childrenChanged()
{
    sequences_ = findChildObjects<Sequence>(QString(), true);
}

} // namespace MO
