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

void SoundSource::performAudioBlock(SamplePos pos, uint thread)
{
    // copy the block of transformations
    audio_->setTransformation(transformations(thread), thread);

    // copy the samples from the audiotrack

    audioTrack_->getValues(pos, sampleRateInv(), bufferSize(thread),
                           audio_->getSamples(thread));

    /*
    for (uint i=0; i<bufferSize(thread); ++i)
    {
        const int sec = (pos + i) / sampleRate();
        const int sam = (pos + i) % sampleRate();
        audio_->setSample(0.5*sin((sec + sam * sampleRateInv())*6.28*437.0), thread, i);
    }
    */
}


} // namespace MO
