/** @file audioinao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.12.2014</p>
*/

#include "audioinao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "math/constants.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(AudioInAO)

AudioInAO::AudioInAO(QObject *parent)
    : AudioObject   (parent)
{
    setName("AudioIn");
}

void AudioInAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aoin", 1);
}

void AudioInAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aoin", 1);
}

void AudioInAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("in", tr("input"));

        paramAmp_ = params()->createFloatParameter("amp", tr("amplitude"),
                                                   tr("The amplitude of the audio input"),
                                                   1.0, 0.05);
    params()->endParameterGroup();
}

void AudioInAO::processAudio(const QList<AUDIO::AudioBuffer *> &inputs,
                             const QList<AUDIO::AudioBuffer *> &outputs,
                             uint bsize, SamplePos pos, uint thread)
{
    // simply copy inputs to outputs here and apply amplitude

    if (paramAmp_->isModulated())
    {
        AUDIO::AudioBuffer::process(inputs, outputs,
        [=](const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            for (SamplePos i=0; i<bsize; ++i)
            {
                F32 amp = paramAmp_->value(sampleRateInv() * (pos + i), thread);
                out->write(i, amp * in->read(i));
            }
        });
    }
    else
    {
        // buffer parameter value
        const F32 amp = paramAmp_->value(sampleRateInv() * pos, thread);
        // for easier loop
        AUDIO::AudioBuffer::process(inputs, outputs,
        [=](const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            for (SamplePos i=0; i<bsize; ++i)
                out->write(i, amp * in->read(i));
        });
    }
}

} // namespace MO
