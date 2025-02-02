/** @file sequence.h

    @brief Abstract sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_SEQUENCE_H
#define MOSRC_OBJECT_SEQUENCE_H

#include <QList>
#include <QColor>

#include "object/Object.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "object/control/Clip.h"
#include "math/functions.h"

namespace MO {

class Sequence : public Object
{
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(Sequence)

    bool isSequence() const { return true; }

    virtual QString infoName() const;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void onParentChanged() Q_DECL_OVERRIDE;

    // -------------- parents -------------------

    /** The track, this sequence is on, or NULL */
    Track * parentTrack() const;

    /** The clip, this sequence is on, or NULL */
    Clip * parentClip() const;

    // --------------- edit --------------------

    /** Splits the sequence at the given time.
        This sequence is shortened and the secand part is created as new Sequence.
        @returns The new created sequence or NULL if @p localTime is out of range. */
    virtual Sequence* splitSequence(Double localTime) = 0;

    // -------------- getter -------------------

    const QColor& color() const { return color_; }

    static Double minimumSpeed() { return 0.001; }
    static Double minimumLength() { return 0.005; }

    /** Start time in seconds */
    Double start() const { return p_start_->baseValue(); }

    /** Return actual start time in seconds,
        including the start time of the parentClip() */
    Double realStart() const { return parentClip_
                ? p_start_->baseValue() + parentClip_->timeStarted()
                : p_start_->baseValue(); }

    /** End time in seconds */
    Double end() const
        { return p_start_->baseValue() + p_length_->baseValue() / p_speed_->baseValue(); }

    /** Length in seconds */
    Double length() const { return p_length_->baseValue(); }

    /** Wheter the sequence is looping or not */
    bool looping() const { return p_looping_->baseValue(); }

    /** Loop start time (local) in seconds */
    Double loopStart() const { return p_loopStart_->baseValue(); }
    Double loopStart(const RenderTime& time) const { return p_loopStart_->value(time); }

    /** Loop length in seconds */
    Double loopLength() const { return p_loopLength_->baseValue(); }
    Double loopLength(const RenderTime& time) const { return p_loopLength_->value(time); }

    /** Loop end time (local) in seconds */
    Double loopEnd() const
        { return (p_loopStart_->baseValue() + p_loopLength_->baseValue()) / p_speed_->baseValue(); }

    /** Offset into the sequence data (local) in seconds. */
    Double timeOffset() const { return p_timeOffset_->baseValue(); }
    Double timeOffset(const RenderTime& time) const { return p_timeOffset_->value(time); }

    /** Sequence internal speed */
    Double speed() const { return p_speed_->baseValue(); }

    // --------------- setter ---------------------

    void setColor(const QColor& color) { color_ = color; }

    void setStart(Double t)
        { p_start_->setValue(std::max(Double(0), t)); }

    void setEnd(Double t)
        { p_length_->setValue(std::max(minimumLength(),
                                       (t - p_start_->baseValue()) * p_speed_->baseValue())); }

    void setLength(Double t)
        { p_length_->setValue(std::max(minimumLength(), t)); }

    void setLooping(bool doit)
        { p_looping_->setValue(doit); }

    void setLoopStart(Double t)
        { p_loopStart_->setValue(t); }

    void setLoopEnd(Double t)
        { p_loopLength_->setValue(std::max(minimumLength(),
                    (t - p_loopStart_->baseValue()) * p_speed_->baseValue())); }

    void setLoopLength(Double t)
        { p_loopLength_->setValue( std::max(minimumLength(), t) ); }

    void setDefaultLoopLength(Double t)
        { p_loopLength_->setDefaultValue( std::max(minimumLength(), t) ); }

    void setTimeOffset(Double t)
        { p_timeOffset_->setValue(t); }

    void setSpeed(Double t)
        { p_speed_->setValue(std::max(minimumSpeed(), t)); }

    /** Translates global time to sequence-local time (with loop) */
    void getSequenceTime(RenderTime & inout_time, Double &timeWithoutLoop) const;

    /** Translates global time to sequence-local time (with loop)
        and returns the current loop settings */
    void getSequenceTime(RenderTime & inout_time,
                           Double& loopStart, Double& loopLength, bool& isInLoop,
                           Double &timeWithoutLoop) const;

signals:

public slots:

private:

    ParameterFloat
        * p_start_,
        * p_length_,
        * p_speed_,
        * p_loopStart_,
        * p_loopLength_,
        * p_timeOffset_;
    ParameterSelect
        * p_looping_;

    /** The track, this sequence is on */
    Track* parentTrack_;
    Clip * parentClip_;

    QColor color_;
};



inline void Sequence::getSequenceTime(RenderTime & rtime, Double &timeWithoutLoop) const
{
    Double speed = p_speed_->value(rtime),
           time = rtime.second();

    if (parentClip_)
    {
        time -= parentClip_->timeStarted();
        speed *= parentClip_->speed();

        rtime.setSecond(time);
        rtime.setSample(time * rtime.sampleRate());
    }

    time = (time - p_start_->value(rtime)) * speed;
    rtime.setSecond(time);
    rtime.setSample(time * rtime.sampleRate());
    time += p_timeOffset_->value(rtime);

    timeWithoutLoop = time;

    if (p_looping_->baseValue())
    {
        const Double
                ls = p_loopStart_->value(rtime),
                ll = std::max(p_loopLength_->value(rtime), minimumLength());

        if (time > ls + ll)
            time = MATH::moduloSigned(time - ls, ll) + ls;
    }

    rtime.setSecond(time);
}

inline void Sequence::getSequenceTime(RenderTime& rtime,
                                        Double& lStart, Double& lLength, bool& isInLoop,
                                        Double& timeWithoutLoop) const
{
    Double speed = p_speed_->value(rtime),
           time = rtime.second();

    if (parentClip_)
    {
        time -= parentClip_->timeStarted();
        speed *= parentClip_->speed();

        rtime.setSecond(time);
        rtime.setSample(time * rtime.sampleRate());
    }

    time = (time - p_start_->value(rtime)) * speed;
    rtime.setSecond(time);
    rtime.setSample(time * rtime.sampleRate());
    time += p_timeOffset_->value(rtime);

    timeWithoutLoop = time;

    if (p_looping_->baseValue())
    {
        lStart = p_loopStart_->value(rtime);
        lLength = std::max(p_loopLength_->value(rtime), minimumLength());

        isInLoop = time >= lStart;

        if (time > lStart + lLength)
        {
            time = MATH::moduloSigned(time - lStart, lLength) + lStart;
        }
    }
    else
        isInLoop = false;

    rtime.setSecond(time);
}

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCE_H
