/** @file audioobject.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include "audioobject.h"
#include "audio/tool/audiobuffer.h"
#include "io/datastream.h"
#include "io/error.h"

namespace MO {

class AudioObject::PrivateAO
{
public:
    PrivateAO()
        : numOutputs    (1)
    { }

    int numOutputs;
};


AudioObject::AudioObject(QObject *parent)
    : Object    (parent),
      p_ao_     (new PrivateAO)
{
}

AudioObject::~AudioObject()
{
    delete p_ao_;
}

void AudioObject::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("ao", 1);
}

void AudioObject::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("ao", 1);
}

uint AudioObject::numAudioOutputs() const
{
    return p_ao_->numOutputs;
}

void AudioObject::setNumberAudioOutputs(uint num)
{
    p_ao_->numOutputs = num;
}


void AudioObject::processAudioBase(const QList<AUDIO::AudioBuffer *> &inputs,
                                   const QList<AUDIO::AudioBuffer *> &outputs,
                                   uint bufferSize, SamplePos pos, uint thread)
{
    MO_ASSERT(outputs.size() == (int)numAudioOutputs(), "output size mismatch "
                        << outputs.size() << "/" << numAudioOutputs());

    // check activity
    if (!active(sampleRateInv() * pos, thread))
    {
        bypass(inputs, outputs);
        return;
    }

#ifdef MO_ENABLE_ASSERT
    // test buffersize
    const int num = std::min(inputs.size(), outputs.size());
    for (int i = 0; i<num; ++i)
        MO_ASSERT(inputs[i]->blockSize() == outputs[i]->blockSize(), "unmatched buffersize "
                  << inputs[i]->blockSize() << "/" << outputs[i]->blockSize());
#endif

    // call virtual function
    processAudio(inputs, outputs, bufferSize, pos, thread);
}


void AudioObject::bypass(const QList<AUDIO::AudioBuffer *> &inputs,
                         const QList<AUDIO::AudioBuffer *> &outputs)
{
    const int num = std::min(inputs.size(), outputs.size());

    // copy inputs
    for (int i = 0; i<num; ++i)
    {
        MO_ASSERT(inputs[i]->blockSize() == outputs[i]->blockSize(), "unmatched buffersize "
                  << inputs[i]->blockSize() << "/" << outputs[i]->blockSize());
        outputs[i]->insert( inputs[i]->readPointer() );
    }

    // clear remaining
    for (int i = num; i < outputs.size(); ++i)
    {
        outputs[i]->insertNullBlock();
    }
}


} // namespace MO
