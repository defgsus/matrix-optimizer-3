/** @file soundsource.cpp

    @brief Basic sound Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "soundsource.h"
#include "io/datastream.h"
#include "audio/tool/audiobuffer.h"
#include "audio/spatial/spatialsoundsource.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "io/log.h"


namespace MO {

MO_REGISTER_OBJECT(SoundSource)

SoundSource::SoundSource(QObject *parent) :
    Object(parent)
{
    setName("Soundsource");
    setNumberSoundSources(1);
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

    params()->beginParameterGroup("audiotrack", tr("audio"));
    initParameterGroupExpanded("audiotrack");

        audioTrack_ = params()->createFloatParameter("audio_track", tr("audio"),
                                           tr("The audio signal of the sound source"),
                                           0.0, false);
    params()->endParameterGroup();
}

void SoundSource::calculateSoundSourceBuffer(
        const QList<AUDIO::SpatialSoundSource *> list,
        uint bufferSize, SamplePos pos, uint thread)
{
    // copy the float parameter into the soundsource buffer
    for (SamplePos i = 0; i < bufferSize; ++i)
        list[0]->signal()->write(i,
                        audioTrack_->value(sampleRateInv() * (pos + i), thread));
}

} // namespace MO
