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

    io << start_ << length_ << speed_ << (qint8)looping_;
}

void Sequence::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("seq", 1);

    qint8 looping;
    io >> start_ >> length_ >> speed_ >> looping;
    looping_ = looping;
}

QString Sequence::infoName() const
{
    if (track_)
        return name() + " (on " + track_->name() + ")";
    else
        return name();
}

Track * Sequence::track() const
{
    return qobject_cast<Track*>(parentObject());
}

void Sequence::createParameters()
{
    loopStart_ = createFloatParameter("loop_start", "loop start", 0.0);
    loopLength_ = createFloatParameter("loop_len", "loop length", 1.0);
    timeOffset_ = createFloatParameter("time_offset", "time offset", 0.0);
}


} // namespace MO
