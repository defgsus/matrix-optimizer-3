/** @file microphoneao.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 14.06.2015</p>
*/

#ifndef MO_DISABLE_SPATIAL

#include "MicrophoneAO.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/spatial/SpatialMicrophone.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
//#include "math/constants.h"
#include "io/DataStream.h"

namespace MO {

MO_REGISTER_OBJECT(MicrophoneAO)

MicrophoneAO::MicrophoneAO()
    : AudioObject   (),
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
        const QList<AUDIO::SpatialMicrophone *> &mics, const RenderTime& time)
{
    const auto outs = audioOutputs(time.thread());
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
            F32 amp = p_amp_->value(time);
            /** @todo cast in stone the block position for Object::processMicrophoneBuffers(),
             * e.g. current or last */
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
