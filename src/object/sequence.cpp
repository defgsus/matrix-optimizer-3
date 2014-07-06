/** @file sequence.cpp

    @brief Abstract sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequence.h"
#include "io/datastream.h"
#include "io/error.h"
#include "track.h"

namespace MO {

Sequence::Sequence(QObject *parent) :
    Object      (parent),
    start_      (0.0),
    length_     (60.0),
    loopStart_  (0.0),
    loopLength_ (length_),
    timeOffset_ (0.0),
    speed_      (1.0),
    looping_    (false),
    track_      (0)
{
    setName("Sequence");
}

void Sequence::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("seq", 1);

    io << start_ << length_ << loopStart_ << loopLength_
       << timeOffset_ << speed_ << (qint8)looping_;
}

void Sequence::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("seq", 1);

    qint8 looping;
    io >> start_ >> length_ >> loopStart_ >> loopLength_
       >> timeOffset_ >> speed_ >> looping;
    looping_ = looping;
}

void Sequence::addToTrack(Track *t)
{
    /*
    MO_ASSERT(!track_, "trying to set track '" << t->idName() << "', but sequence '"
              << idName() << "' already has a track '"
              << track_->idName() << "' assigned");
    */
    track_ = t;
}

void Sequence::removeFromTrack(Track *t)
{
    MO_ASSERT(track_ == t, "trying to remove sequence '" << idName() << "' from track '"
              << t->idName() << "' but assigned track is " << track_);

    track_ = 0;
}

} // namespace MO
