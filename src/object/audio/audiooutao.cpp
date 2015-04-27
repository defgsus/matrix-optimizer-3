/** @file audiooutao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#include "audiooutao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "math/constants.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(AudioOutAO)

AudioOutAO::AudioOutAO(QObject *parent)
    : AudioObject   (parent)
{
    setName("AudioOut");
}

void AudioOutAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aoout", 1);
}

void AudioOutAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aoout", 1);
}

void AudioOutAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("out", tr("output"));
    initParameterGroupExpanded("out");

        paramAmp_ = params()->createFloatParameter("amp", tr("amplitude"),
                                                   tr("The amplitude of the audio output"),
                                                   1.0, 0.1);
    params()->endParameterGroup();
}

void AudioOutAO::processAudio(uint bsize, SamplePos pos, uint thread)
{
    // simply copy inputs to outputs here and apply amplitude

    if (paramAmp_->isModulated())
    {
        AUDIO::AudioBuffer::process(audioInputs(thread), audioOutputs(thread),
        [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
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
        AUDIO::AudioBuffer::process(audioInputs(thread), audioOutputs(thread),
        [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            for (SamplePos i=0; i<bsize; ++i)
                out->write(i, amp * in->read(i));
        });
    }
}

} // namespace MO
