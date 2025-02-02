/** @file audioobject.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include "AudioObject.h"
#include "param/Parameters.h"
#include "param/ParameterInt.h"
#include "util/ObjectEditor.h"
#include "audio/tool/AudioBuffer.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

class AudioObject::PrivateAO
{
public:
    PrivateAO()
        : numInputs         (-1),
          numOutputs        (1),
          channelsAdjustable(false),
          paramChannels     (0)
    { }

    int numInputs;
    uint numOutputs;

    bool channelsAdjustable;

    ParameterInt * paramChannels;

    // audio buffers per thread
    QVector<QList<AUDIO::AudioBuffer*>>
        inputs, outputs;

    QVector<SamplePos> lastOutputSamplePos;
};


AudioObject::AudioObject()
    : Object    (),
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

void AudioObject::setNumberThreads(uint num)
{
    Object::setNumberThreads(num);
    p_ao_->inputs.resize(num);
    p_ao_->outputs.resize(num);
    p_ao_->lastOutputSamplePos.resize(num);
    for (auto & p : p_ao_->lastOutputSamplePos)
        p = 0;
}

void AudioObject::createParameters()
{
    Object::createParameters();

    if (p_ao_->channelsAdjustable)
    {
        params()->beginParameterGroup("_ao_channels", "audio channels");

            p_ao_->paramChannels = params()->createIntParameter("_ao_insouts",
                                                            tr("Number of channels"),
                                                            tr("Selects the desired number of input and output channels"),
                                                            1, 1, false);
            p_ao_->paramChannels->setMinValue(1);

        params()->endParameterGroup();
    }
}

void AudioObject::onParameterChanged(Parameter * p)
{
    Object::onParameterChanged(p);

    if (p_ao_->channelsAdjustable && p == p_ao_->paramChannels)
        setNumberAudioInputsOutputs(p_ao_->paramChannels->baseValue(),
                                    p_ao_->paramChannels->baseValue());
}

void AudioObject::onParametersLoaded()
{
    Object::onParametersLoaded();

    if (p_ao_->channelsAdjustable)
        setNumberAudioInputsOutputs(p_ao_->paramChannels->baseValue(),
                                    p_ao_->paramChannels->baseValue());
}

void AudioObject::setNumberChannelsAdjustable(bool e)
{
    // XXX should have a flag in object that signals if object construction has finished
    // so we could assert here that this function is called before the creation of parameters
    p_ao_->channelsAdjustable = e;
}

uint AudioObject::numAudioOutputs() const
{
    return p_ao_->numOutputs;
}

int AudioObject::numAudioInputs() const
{
    return p_ao_->numInputs;
}

void AudioObject::setNumberAudioOutputs(uint num, bool emitSignal)
{
    p_ao_->numOutputs = num;

    setNumberOutputs(ST_AUDIO, num, emitSignal);

    //if (changed && emitSignal && editor())
    //    editor()->emitAudioChannelsChanged(this);
}

void AudioObject::setNumberAudioInputs(int num, bool emitSignal)
{
    bool changed = p_ao_->numInputs != num;

    p_ao_->numInputs = num;

    if (changed && emitSignal && editor())
        editor()->emitAudioChannelsChanged(this);
}

void AudioObject::setNumberAudioInputsOutputs(uint numIn, uint numOut, bool emitSignal)
{
    bool changed = p_ao_->numOutputs != numIn
            || p_ao_->numInputs != (int)numOut;

    p_ao_->numInputs = numIn;
    p_ao_->numOutputs = numOut;
    setNumberOutputs(ST_AUDIO, numOut, emitSignal);

    if (changed && emitSignal && editor())
        editor()->emitAudioChannelsChanged(this);
}


void AudioObject::setAudioBuffersBase(uint thread, uint bufferSize,
                                      const QList<AUDIO::AudioBuffer *> &inputs,
                                      const QList<AUDIO::AudioBuffer *> &outputs)
{
    MO_ASSERT((int)thread < p_ao_->inputs.size(), "thread mismatch " << thread << "/" <<
                                                    p_ao_->inputs.size());
    // copy buffers
    p_ao_->inputs[thread] = inputs;
    p_ao_->outputs[thread] = outputs;

    setAudioBuffers(thread, bufferSize, inputs, outputs);
}

const QList<AUDIO::AudioBuffer*> & AudioObject::audioInputs(uint thread) const
{
    MO_ASSERT((int)thread < p_ao_->inputs.size(), "thread mismatch " << thread << "/" <<
                                                    p_ao_->inputs.size());
    return p_ao_->inputs[thread];
}

const QList<AUDIO::AudioBuffer*> & AudioObject::audioOutputs(uint thread) const
{
    MO_ASSERT((int)thread < p_ao_->outputs.size(), "thread mismatch " << thread << "/" <<
                                                    p_ao_->outputs.size());
    return p_ao_->outputs[thread];
}

void AudioObject::processAudioBase(const RenderTime & time)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs = audioInputs(time.thread()),
            outputs = audioOutputs(time.thread());

/*    MO_ASSERT(outputs.size() == (int)numAudioOutputs(), "output size mismatch "
                        << outputs.size() << "/" << numAudioOutputs());
*/
    // check activity
    if (!active(time))
    {
        AUDIO::AudioBuffer::bypass(inputs, outputs);
        return;
    }

#ifdef MO_ENABLE_ASSERT
    // test buffersize
    const int num = std::min(inputs.size(), outputs.size());
    for (int i = 0; i<num; ++i)
        if (inputs[i] && outputs[i])
            MO_ASSERT(inputs[i]->blockSize() == outputs[i]->blockSize(), "unmatched buffersize "
                  << inputs[i]->blockSize() << "/" << outputs[i]->blockSize());
#endif

    // call virtual function
    try
    {
        processAudio(time);
    }
    catch (const Exception& e)
    {
        setErrorMessage(e.what());
    }

    p_ao_->lastOutputSamplePos[time.thread()] = time.sample() + time.bufferSize() - 1;
}

void AudioObject::clientFakeAudio(const RenderTime& time)
{
    p_ao_->lastOutputSamplePos[time.thread()]
            = time.sample() + time.bufferSize() - 1;
}


Double AudioObject::getAudioOutputAsFloat(uint channel, const RenderTime & time) const
{
    auto& outs = audioOutputs(time.thread());

    if ((int)channel >= outs.size() || outs[channel] == nullptr)
        return 0.;

    SamplePosDiff pos =
              p_ao_->lastOutputSamplePos[time.thread()]
              - SamplePosDiff(time.sample());
                 //- outs[channel]->blockSize() * (outs[channel]->numBlocks() - 1));
    //MO_DEBUG("ACCESS " << outs[channel] << " AT " << pos);
    // repeat first sample in current read block
    if (pos < 0)
        return outs[channel]->read(0);

    return outs[channel]->readHistory(pos);
}

void AudioObject::writeNullBlock(SamplePos , uint thread)
{
    const auto & outputs = audioOutputs(thread);

    if (outputs.isEmpty())
        return;

    // XXX Could be smart here using samplepos
    // and stop filling buffers after
    // AUDIO::AudioBuffer::numBlocks() is filled

    for (auto o : outputs)
        if (o)
            o->writeNullBlock();
}



} // namespace MO
