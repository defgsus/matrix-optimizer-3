/** @file soundsource.cpp

    @brief Basic sound Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MO_DISABLE_SPATIAL

#include "SoundSource.h"
#include "io/DataStream.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/spatial/SpatialSoundSource.h"
#include "param/Parameters.h"
#include "param/ParameterFloat.h"
#include "io/log.h"


namespace MO {

MO_REGISTER_OBJECT(SoundSource)

SoundSource::SoundSource()
    : Object()
{
    setName("Soundsource");
    setNumberSoundSources(1);
}

SoundSource::~SoundSource() { }

void SoundSource::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("snd", 1);
}

void SoundSource::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("snd", 1);
}

void SoundSource::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("audiotrack", tr("audio"));
    initParameterGroupExpanded("audiotrack");

        audioTrack_ = params()->createFloatParameter("audio_track", tr("audio"),
                                           tr("The audio signal of the sound source"),
                                           0.0, false);
    params()->endParameterGroup();
}

void SoundSource::calculateSoundSourceBuffer(
        const QList<AUDIO::SpatialSoundSource *> list,
        const RenderTime& time)
{
    RenderTime t(time);
    // copy the float parameter into the soundsource buffer
    for (SamplePos i = 0; i < time.bufferSize(); ++i)
    {
        t.setSample(time.sample() + i);
        t.setSecond(sampleRateInv() * t.sample());
        list[0]->signal()->write(i, audioTrack_->value(t));
    }
}

} // namespace MO

#endif
