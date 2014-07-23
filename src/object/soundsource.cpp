/** @file soundsource.cpp

    @brief Basic sound Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "soundsource.h"
#include "io/datastream.h"
#include "audio/audiosource.h"
#include "param/parameterfloat.h"

namespace MO {

MO_REGISTER_OBJECT(SoundSource)

SoundSource::SoundSource(QObject *parent) :
    Object(parent)
{
    setName("Soundsource");
}

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

    audioTrack_ = createFloatParameter("audio_track", "audio",
                                       tr("The audio signal of the sound source"),
                                       0.0, false);
}

void SoundSource::createAudioSources()
{
    Object::createAudioSources();

    audio_ = createAudioSource();
}

void SoundSource::sampleStep(Double time, int thread)
{
    audio_->setTransformation(transformation(thread), thread);
    audio_->setSample(audioTrack_->value(time), thread);
}


} // namespace MO
