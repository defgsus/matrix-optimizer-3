/** @file soundsourceao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#ifndef MO_DISABLE_SPATIAL

#include "soundsourceao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/spatial/spatialsoundsource.h"
//#include "object/param/parameters.h"
//#include "object/param/parameterfloat.h"
//#include "math/constants.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(SoundSourceAO)

SoundSourceAO::SoundSourceAO()
    : AudioObject   ()
{
    setName("SoundSource");
    setNumberAudioInputsOutputs(1,1);
    setNumberSoundSources(1);
}

SoundSourceAO::~SoundSourceAO() { }

void SoundSourceAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aosrc", 1);
}

void SoundSourceAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aosrc", 1);
}

void SoundSourceAO::createParameters()
{
    AudioObject::createParameters();
/*
    params()->beginParameterGroup("in", tr("input"));

        paramAmp_ = params()->createFloatParameter("amp", tr("amplitude"),
                                                   tr("The amplitude of the audio input"),
                                                   1.0, 0.05);
    params()->endParameterGroup();
*/
}

void SoundSourceAO::calculateSoundSourceBuffer(
                            const QList<AUDIO::SpatialSoundSource *> snds,
                            const RenderTime& time)
{
    // simply copy input channel
    if (!audioInputs(time.thread()).isEmpty())
        if (const auto in = audioInputs(time.thread())[0])
            snds[0]->signal()->writeBlock(in->readPointer());
}


} // namespace MO

#endif
