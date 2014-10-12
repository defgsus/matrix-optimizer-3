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
#include "clip.h"

namespace MO {

Sequence::Sequence(QObject *parent) :
    Object          (parent),
    parentTrack_    (0),
    parentClip_     (0)
{
    setName("Sequence");
}

void Sequence::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("seq", 2);

}

void Sequence::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    const int ver = io.readHeader("seq", 2);

    if (ver <= 1)
        MO_IO_ERROR(VERSION_MISMATCH, "Can't read sequence format prior to version 2");
}

QString Sequence::infoName() const
{
    if (parentTrack_)
        return name() + " (on " + parentTrack_->name() + ")";
    else if (parentClip_)
        return name() + " (on " + parentClip_->name() + ")";
    else
        return name();
}

void Sequence::onParentChanged()
{
    Object::onParentChanged();

    parentTrack_ = qobject_cast<Track*>(parentObject());
    parentClip_ = qobject_cast<Clip*>(parentObject());
}

Track * Sequence::parentTrack() const
{
    return parentTrack_;
}

Clip * Sequence::parentClip() const
{
    return parentClip_;
}

void Sequence::createParameters()
{
    Object::createParameters();

    beginParameterGroup("time", tr("time"));

        p_start_ = createFloatParameter("start", tr("sequence start"),
                                          tr("Global start time of the sequence in seconds"),
                                          0.0);
        p_start_->setMinValue(0.0);

        p_length_ = createFloatParameter("length", tr("sequence length"),
                                          tr("Length of the sequence in seconds"),
                                          60.0);
        p_length_->setMinValue(minimumLength());

        p_timeOffset_ = createFloatParameter("time_offset", tr("time offset"),
                                           tr("Time offset into the sequence data in seconds"),
                                           0.0);

        p_speed_ = createFloatParameter("speed", tr("speed"),
                                          tr("Time multiplier for the whole sequence"),
                                          1.0);
        p_speed_->setMinValue(minimumSpeed());

    endParameterGroup();

    beginParameterGroup("loop", "looping");

        p_looping_ = createBooleanParameter("looping", tr("looping"),
                                          tr("Enables an internal loop for the sequence"),
                                          tr("No looping"), tr("Looping enabled"),
                                          false, true, false);

        p_loopStart_ = createFloatParameter("loop_start", tr("loop start"),
                                          tr("Local start time of the loop in seconds"),
                                          0.0);

        p_loopLength_ = createFloatParameter("loop_len", tr("loop length"),
                                           tr("Length of loop in seconds"),
                                           1.0);
        p_loopLength_->setMinValue(minimumLength());

    endParameterGroup();
}


void Sequence::updateParameterVisibility()
{
    Object::updateParameterVisibility();

    const bool loop = p_looping_->baseValue();
    p_loopStart_->setVisible(loop);
    p_loopLength_->setVisible(loop);
}

} // namespace MO
