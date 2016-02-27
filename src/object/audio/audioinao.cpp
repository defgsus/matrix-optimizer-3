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

AudioInAO::AudioInAO()
    : AudioObject   ()
{
    setName("AudioIn");
    setNumberAudioOutputs(2);
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
    initParameterGroupExpanded("in");

        paramAmp_ = params()->createFloatParameter("amp", tr("amplitude"),
                                                   tr("The amplitude of the audio input"),
                                                   1.0, 0.05);
    params()->endParameterGroup();
}

void AudioInAO::processAudio(const RenderTime& time)
{
    // simply copy inputs to outputs here and apply amplitude

    if (paramAmp_->isModulated())
    {
        AUDIO::AudioBuffer::process(audioInputs(time.thread()), audioOutputs(time.thread()),
        [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            for (SamplePos i=0; i<time.bufferSize(); ++i)
            {
                F32 amp = paramAmp_->value(time + i);
                out->write(i, amp * in->read(i));
            }
        });
    }
    else
    {
        // buffer parameter value
        const F32 amp = paramAmp_->value(time);
        // for easier loop
        AUDIO::AudioBuffer::process(audioInputs(time.thread()), audioOutputs(time.thread()),
        [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            for (SamplePos i=0; i<time.bufferSize(); ++i)
                out->write(i, amp * in->read(i));
        });
    }
}

} // namespace MO
