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
#include "param/parameterfloat.h"

namespace MO {

class Sequence : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(Sequence)

    bool isSequence() const { return true; }

    virtual QString infoName() const;

    virtual void createParameters();

    // -------------- tracks -------------------

    /** The track, this sequence is on (actually the parent) */
    Track * track() const;

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
    Double loopStart() const { return loopStart_->baseValue(); }

    /** Loop length in seconds */
    Double loopLength() const { return loopLength_->baseValue(); }

    /** Loop end time (local) in seconds */
    Double loopEnd() const { return (loopStart_->baseValue() + loopLength_->baseValue()) / speed_; }

    /** Offset into the sequence data (local) in seconds. */
    Double timeOffset() const { return timeOffset_->baseValue(); }

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
        { loopStart_->setValue(t); if (send_signal) emit timeChanged(this); }

    void setLoopEnd(Double t, bool send_signal = true)
        { loopLength_->setValue(std::max(minimumLength(), (t - loopStart_->baseValue()) * speed_));
          if (send_signal) emit timeChanged(this); }

    void setLoopLength(Double t, bool send_signal = true)
        { loopLength_->setValue( std::max(minimumLength(), t) );
          if (send_signal) emit timeChanged(this); }

    void setTimeOffset(Double t, bool send_signal = true)
        { timeOffset_->setValue(t); if (send_signal) emit timeChanged(this); }

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
           speed_;
    bool   looping_;

    ParameterFloat
        * loopStart_,
        * loopLength_,
        * timeOffset_;

    /** The track, this sequence is on */
    Track* track_;
};


inline Double Sequence::getSequenceTime(Double time) const
{
    time = (time - start_) * speed_;
    time += timeOffset_->value(time);

    if (looping_)
    {
        const Double
                ls = loopStart_->value(time),
                ll = std::max(loopLength_->value(time), minimumLength());

        if (time > ls + ll)
            return MATH::moduloSigned(time - ls, ll);
    }

    return time;
}

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCE_H
