/** @file sequence.h

    @brief Abstract sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_SEQUENCE_H
#define MOSRC_OBJECT_SEQUENCE_H

#include <QList>

#include "object.h"
#include "math/functions.h"

namespace MO {

class Sequence : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(Sequence)

    bool isSequence() const { return true; }

    // -------------- tracks -------------------

    /** The track, this sequence is on */
    Track * track() const { return track_; }

    /** Only changes the track() property.
        Tracks are responsible for knowing their Sequences
        and for telling them! */
    void addToTrack(Track * t);
    void removeFromTrack(Track * t);

    // -------------- getter -------------------

    static Double minimumSpeed() { return 0.001; }
    static Double minimumLength() { return 0.005; }

    /** Start time in seconds */
    Double start() const { return start_; }

    /** End time in seconds */
    Double end() const { return start_ + length_ / speed_; }

    /** Length in seconds */
    Double length() const { return length_; }

    /** Wheter the sequence is looping or not */
    bool looping() const { return looping_; }

    /** Loop start time (local) in seconds */
    Double loopStart() const { return loopStart_; }

    /** Loop length in seconds */
    Double loopLength() const { return loopLength_; }

    /** Loop end time (local) in seconds */
    Double loopEnd() const { return (loopStart_ + loopLength_) / speed_; }

    /** Offset into the sequence data (local) in seconds. */
    Double timeOffset() const { return timeOffset_; }

    /** Sequence internal speed */
    Double speed() const { return speed_; }

    // --------------- setter ---------------------

    void setStart(Double t, bool send_signal = true)
        { start_ = t; if (send_signal) emit timeChanged(this); }

    void setEnd(Double t, bool send_signal = true)
        { length_ = std::max(minimumLength(), (t - start_) * speed_);
          if (send_signal) emit timeChanged(this); }

    void setLength(Double t, bool send_signal = true)
        { length_ = std::max(minimumLength(), t); if (send_signal) emit timeChanged(this); }

    void setLooping(bool doit, bool send_signal = true)
        { looping_ = doit; if (send_signal) emit timeChanged(this); }

    void setLoopStart(Double t, bool send_signal = true)
        { loopStart_ = t; if (send_signal) emit timeChanged(this); }

    void setLoopEnd(Double t, bool send_signal = true)
        { loopLength_ = std::max(minimumLength(), (t - loopStart_) * speed_);
          if (send_signal) emit timeChanged(this); }

    void setLoopLength(Double t, bool send_signal = true)
        { loopLength_ = std::max(minimumLength(), t);
          if (send_signal) emit timeChanged(this); }

    void setTimeOffset(Double t, bool send_signal = true)
        { timeOffset_ = t; if (send_signal) emit timeChanged(this); }

    void setSpeed(Double t, bool send_signal = true)
        { speed_ = (t >= minimumSpeed()) ? t : minimumSpeed();
          if (send_signal) emit timeChanged(this); }

    /** Translates global time to sequence-local time */
    Double getSequenceTime(Double global_time) const;

signals:

    /** Emitted, when any of the time or loop settings changed */
    void timeChanged(MO::Sequence *);

public slots:

private:

    Double start_,
           length_,
           loopStart_,
           loopLength_,
           timeOffset_,
           speed_;
    bool   looping_;

    /** The track, this sequence is on */
    Track* track_;
};


inline Double Sequence::getSequenceTime(Double time) const
{
    time = (time - start_) * speed_ + timeOffset_;

    if (looping_ && time > loopStart_ + loopLength_)

        return MATH::moduloSigned(time - loopStart_, loopLength_);

    else

        return time;
}

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCE_H
