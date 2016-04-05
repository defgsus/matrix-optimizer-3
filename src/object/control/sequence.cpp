/** @file sequence.cpp

    @brief Abstract sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequence.h"
#include "object/param/parameters.h"
#include "object/control/clip.h"
#include "track.h"
#include "io/datastream.h"
#include "io/error.h"

namespace MO {

Sequence::Sequence() :
    Object          (),
    parentTrack_    (0),
    parentClip_     (0),
    color_          (QColor(80, 120, 80))
{
    setName("*Sequence*");
}

Sequence::~Sequence()
{

}

void Sequence::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("seq", 3);

    // v3
    io << color_;
}

void Sequence::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    const int ver = io.readHeader("seq", 3);

    if (ver <= 1)
        MO_IO_ERROR(VERSION_MISMATCH, "Can't read sequence format prior to version 2");

    if (ver >= 3)
        io >> color_;
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

    parentTrack_ = dynamic_cast<Track*>(parentObject());
    parentClip_ = dynamic_cast<Clip*>(parentObject());
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

    params()->beginParameterGroup("time", tr("time"));
    params()->beginEvolveGroup(false);

        p_start_ = params()->createFloatParameter("start", tr("sequence start"),
                                          tr("Global start time of the sequence in seconds"),
                                          0.0);
        p_start_->setMinValue(0.0);

        p_length_ = params()->createFloatParameter("length", tr("sequence length"),
                                          tr("Length of the sequence in seconds"),
                                          60.0);
        p_length_->setMinValue(minimumLength());

        p_timeOffset_ = params()->createFloatParameter("time_offset", tr("time offset"),
                                           tr("Time offset into the sequence data in seconds"),
                                           0.0);

        p_speed_ = params()->createFloatParameter("speed", tr("speed"),
                                          tr("Time multiplier for the whole sequence"),
                                          1.0);
        p_speed_->setMinValue(minimumSpeed());

    params()->endEvolveGroup();
    params()->endParameterGroup();

    params()->beginParameterGroup("loop", "looping");
    params()->beginEvolveGroup(false);

        p_looping_ = params()->createBooleanParameter("looping", tr("looping"),
                                          tr("Enables an internal loop for the sequence"),
                                          tr("No looping"), tr("Looping enabled"),
                                          false, true, false);

        p_loopStart_ = params()->createFloatParameter("loop_start", tr("loop start"),
                                          tr("Local start time of the loop in seconds"),
                                          0.0);

        p_loopLength_ = params()->createFloatParameter("loop_len", tr("loop length"),
                                           tr("Length of loop in seconds"),
                                           1.0);
        p_loopLength_->setMinValue(minimumLength());

    params()->endEvolveGroup();
    params()->endParameterGroup();
}


void Sequence::updateParameterVisibility()
{
    Object::updateParameterVisibility();

    const bool loop = p_looping_->baseValue();
    p_loopStart_->setVisible(loop);
    p_loopLength_->setVisible(loop);
}


} // namespace MO
