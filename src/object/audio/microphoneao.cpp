/** @file microphoneao.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 14.06.2015</p>
*/

#ifndef MO_DISABLE_SPATIAL

#include "microphoneao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/spatial/spatialmicrophone.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
//#include "math/constants.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(MicrophoneAO)

MicrophoneAO::MicrophoneAO(QObject *parent)
    : AudioObject   (parent),
      mbuf_         (new AUDIO::AudioBuffer)
{
    setName("Microphone");
    setNumberAudioInputsOutputs(0, 1);
    setNumberMicrophones(1);
}

MicrophoneAO::~MicrophoneAO()
{
    delete mbuf_;
}

void MicrophoneAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aomic", 1);
}

void MicrophoneAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aomic", 1);
}

void MicrophoneAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("micro", tr("microphone"));

        p_amp_ = params()->createFloatParameter("amp", tr("amplitude"),
                                                   tr("The amplitude of the microphone output"),
                                                   1.0, 0.05);
    params()->endParameterGroup();

}

void MicrophoneAO::processMicrophoneBuffers(
        const QList<AUDIO::SpatialMicrophone *> &mics, SamplePos pos, uint thread)
{
    const auto outs = audioOutputs(thread);
    if (outs.isEmpty())
        return;
    AUDIO::AudioBuffer * out = outs[0];

    if (mics.isEmpty())
        out->writeNullBlock();
    else
    {
        AUDIO::SpatialMicrophone * mic = mics[0];
        if (out->blockSize() == mic->signal()->blockSize())
        {
            F32 amp = p_amp_->value(sampleRateInv() * pos, thread);
            /** @todo cast in stone the block position for Object::processMicrophoneBuffers(), e.g. current or last */
            out->writeBlockMul(mic->signal()->writePointer(), amp);
        }
    }
}

/*
void MicrophoneAO::processAudio(uint bsize, SamplePos pos, uint thread)
{

}*/



} // namespace MO

#endif
