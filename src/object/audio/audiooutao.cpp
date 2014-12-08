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
    setNumberAudioInputs(2);
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
    params()->beginParameterGroup("out", tr("output"));

        paramAmp_ = params()->createFloatParameter("amp", tr("amplitude"),
                                                   tr("The amplitude of the audio output"),
                                                   1.0, 0.1);
    params()->endParameterGroup();
}

void AudioOutAO::processAudio(const QList<AUDIO::AudioBuffer *> &inputs,
                              const QList<AUDIO::AudioBuffer *> &outputs,
                              uint bsize, SamplePos pos, uint thread)
{
    // copy inputs
    AUDIO::AudioBuffer::process(inputs, outputs,
    [=](const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        for (SamplePos i=0; i<bsize; ++i)
        {
            F32 amp = paramAmp_->value(sampleRateInv() * (pos + i), thread);
            out->write(i, amp * in->read(i));
        }
    });
    /*
    for (int i = 0; i<outputs.size(); ++i)
    if (outputs[i])
    {
        if (i < inputs.size() && inputs[i])
            // XXX missing amplitude here
            outputs[i]->writeBlock( inputs[i]->readPointer() );
        else
            outputs[i]->writeNullBlock();
    }*/
}

} // namespace MO
