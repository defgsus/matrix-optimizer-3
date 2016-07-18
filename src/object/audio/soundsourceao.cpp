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
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
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

    params()->beginParameterGroup("in", tr("input"));

        paramAmp_ = params()->createFloatParameter(
                    "amp", tr("amplitude"),
                    tr("The amplitude of the sound source"),
                    1.0, 0.05);

        paramHasDist_ = params()->createBooleanParameter(
                    "has_dist", tr("distance input"),
                    tr("Enables a second input for the audio that "
                       "should be emitted by distance sound sources"),
                    tr("Off"), tr("On"), false, true, false);

    params()->endParameterGroup();

}

void SoundSourceAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    uint n = paramHasDist_->baseValue() ? 2 : 1;
    setNumberAudioInputsOutputs(n,n,true);
}

void SoundSourceAO::onParameterChanged(Parameter* p)
{
    AudioObject::onParameterChanged(p);
    if (p == paramHasDist_)
    {
        uint n = paramHasDist_->baseValue() ? 2 : 1;
        setNumberAudioInputsOutputs(n,n,true);
    }
}

void SoundSourceAO::calculateSoundSourceBuffer(
                            const QList<AUDIO::SpatialSoundSource *> snds,
                            const RenderTime& time)
{
    F32 amp = paramAmp_->value(time);

    // copy input channel(s) to SpatialSoundSource buffers
    auto inputs = audioInputs(time.thread());
    if (!inputs.isEmpty())
    {
        if (const auto in = inputs[0])
        {
            if (amp == 1.f)
                snds[0]->signal()->writeBlock(in->readPointer());
            else
                snds[0]->signal()->writeBlockMul(in->readPointer(), amp);
        }
            else snds[0]->signal()->writeNullBlock();

        if (paramHasDist_->baseValue() && inputs.size() > 1 && inputs[1])
        {
            snds[0]->setDistanceSound(true);
            if (amp == 1.f)
                snds[0]->signalDist()->writeBlock(inputs[1]->readPointer());
            else
                snds[0]->signalDist()->writeBlockMul(inputs[1]->readPointer(), amp);
        }
        else
            snds[0]->setDistanceSound(false);
    }
}


} // namespace MO

#endif
