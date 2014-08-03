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
    bool looping() const { return isLooping_; }

    /** Loop start time (local) in seconds */
    Double loopStart() const { return loopStart_->baseValue(); }
    Double loopStart(Double time, uint thread) const { return loopStart_->value(time, thread); }

    /** Loop length in seconds */
    Double loopLength() const { return loopLength_->baseValue(); }
    Double loopLength(Double time, uint thread) const { return loopLength_->value(time, thread); }

    /** Loop end time (local) in seconds */
    Double loopEnd() const { return (loopStart_->baseValue() + loopLength_->baseValue()) / speed_; }

    /** Offset into the sequence data (local) in seconds. */
    Double timeOffset() const { return timeOffset_->baseValue(); }
    Double timeOffset(Double time, uint thread) const { return timeOffset_->value(time, thread); }

    /** Sequence internal speed */
    Double speed() const { return speed_; }

    // --------------- setter ---------------------

    void setStart(Double t)
        { start_ = t; }

    void setEnd(Double t)
        { length_ = std::max(minimumLength(), (t - start_) * speed_); }

    void setLength(Double t)
        { length_ = std::max(minimumLength(), t); }

    void setLooping(bool doit)
        { isLooping_ = doit; }

    void setLoopStart(Double t)
        { loopStart_->setValue(t); }

    void setLoopEnd(Double t)
        { loopLength_->setValue(std::max(minimumLength(), (t - loopStart_->baseValue()) * speed_)); }

    void setLoopLength(Double t)
        { loopLength_->setValue( std::max(minimumLength(), t) ); }

    void setTimeOffset(Double t)
        { timeOffset_->setValue(t); }

    void setSpeed(Double t)
        { speed_ = (t >= minimumSpeed()) ? t : minimumSpeed(); }

    /** Translates global time to sequence-local time (with loop) */
    Double getSequenceTime(Double global_time, uint thread) const;
    /** Translates global time to sequence-local time and returnd the current loop settings */
    Double getSequenceTime(Double global_time, uint thread,
                           Double& loopStart, Double& loopLength, bool& isInLoop) const;

signals:

public slots:

private:

    Double start_,
           length_,
           speed_;
    bool   isLooping_;

    ParameterFloat
        * loopStart_,
        * loopLength_,
        * timeOffset_;

    /** The track, this sequence is on */
    Track* track_;
};


inline Double Sequence::getSequenceTime(Double time, uint thread) const
{
    time = (time - start_) * speed_;
    time += timeOffset_->value(time, thread);

    if (isLooping_)
    {
        const Double
                ls = loopStart_->value(time, thread),
                ll = std::max(loopLength_->value(time, thread), minimumLength());

        if (time > ls + ll)
            return MATH::moduloSigned(time - ls, ll) + ls;
    }

    return time;
}

inline Double Sequence::getSequenceTime(Double time, uint thread,
                                        Double& lStart, Double& lLength, bool& isInLoop) const
{
    time = (time - start_) * speed_;
    time += timeOffset_->value(time, thread);

    if (isLooping_)
    {
        lStart = loopStart_->value(time, thread);
        lLength = std::max(loopLength_->value(time, thread), minimumLength());

        isInLoop = time >= lStart;

        if (time > lStart + lLength)
        {
            return MATH::moduloSigned(time - lStart, lLength) + lStart;
        }
    }

    isInLoop = false;
    return time;
}

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCE_H
