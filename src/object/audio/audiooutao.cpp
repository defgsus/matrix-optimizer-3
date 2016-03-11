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

AudioOutAO::AudioOutAO()
    : AudioObject   ()
{
    setName("AudioOut");
}

AudioOutAO::~AudioOutAO() { }


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

void AudioOutAO::processAudio(const RenderTime& time)
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
