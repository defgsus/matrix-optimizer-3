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
    Object          (parent),
    tmp_read_ver1_  (false),
    track_          (0)
{
    setName("Sequence");
}

void Sequence::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("seq", 2);

#if (0) // VERSION_1
    io << start_ << length_ << speed_ << (qint8)isLooping_;
#endif
}

void Sequence::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    const int ver = io.readHeader("seq", 2);

    if (ver <= 1)
    {
        tmp_read_ver1_ = true;

        qint8 looping;
        io >> tmp_start_ >> tmp_length_ >> tmp_speed_ >> looping;
        tmp_isLooping_ = looping;
    }
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

void Sequence::onParametersLoaded()
{
    Object::onParametersLoaded();

    if (tmp_read_ver1_)
    {
        tmp_read_ver1_ = false;

        p_start_->setValue(tmp_start_);
        p_length_->setValue(tmp_length_);
        p_speed_->setValue(tmp_speed_);
        p_looping_->setValue(tmp_isLooping_);
    }
}

} // namespace MO
