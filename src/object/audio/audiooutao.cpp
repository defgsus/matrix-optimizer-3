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
    setAudioOutputsVisible(false);
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
                              uint , SamplePos , uint )
{
    const int num = std::min(inputs.size(), outputs.size());

    // copy inputs
    for (int i = 0; i<num; ++i)
    {
        // XXX missing amplitude here
        outputs[i]->writeBlock( inputs[i]->readPointer() );
    }

    // clear remaining
    for (int i = num; i < outputs.size(); ++i)
    {
        outputs[i]->writeNullBlock();
    }
}

} // namespace MO
