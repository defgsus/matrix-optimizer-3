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

Clip::Clip(QObject *parent)
    : Object        (parent),
      clipContainer_(0),
      timeStarted_  (0),
      running_      (false),
      column_       (0),
      row_          (0)
{
    setName("Clip");
}

void Clip::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("clip", 1);

    io << column_ << row_;
}

void Clip::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("clip", 1);

    io >> column_ >> row_;
}

void Clip::createParameters()
{
    Object::createParameters();
}


void Clip::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}

void Clip::onParentChanged()
{
    Object::onParentChanged();

    clipContainer_ = qobject_cast<ClipContainer*>(parentObject());

    if (clipContainer_)
        clipContainer_->findNextFreeSlot(column_, row_, true);
}

void Clip::childrenChanged()
{
    // get all sequences and sub-sequences
    sequences_ = findChildObjects<Sequence>(QString(), true);
}


void Clip::startClip(Double gtime)
{
    timeStarted_ = gtime;
    running_ = true;
}

void Clip::stopClip()
{
    running_ = false;
}


} // namespace MO
