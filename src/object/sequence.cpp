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
    isLooping_    (false),
    track_      (0)
{
    setName("Sequence");
}

void Sequence::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("seq", 1);

    io << start_ << length_ << speed_ << (qint8)isLooping_;
}

void Sequence::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("seq", 1);

    qint8 looping;
    io >> start_ >> length_ >> speed_ >> looping;
    isLooping_ = looping;
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
    Object::createParameters();

    loopStart_ = createFloatParameter("loop_start", "loop start",
                                      tr("Local start time of the loop in seconds"),
                                      0.0);
    loopStart_->setEditable(false);

    loopLength_ = createFloatParameter("loop_len", "loop length",
                                       tr("Length of loop in seconds"),
                                       1.0);
    loopLength_->setEditable(false);

    timeOffset_ = createFloatParameter("time_offset", "time offset",
                                       tr("Time offset into the sequence data in seconds"),
                                       0.0);
    timeOffset_->setEditable(false);
}


} // namespace MO
