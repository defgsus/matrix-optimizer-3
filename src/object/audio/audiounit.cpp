/** @file audiounit.cpp

    @brief Abstract audio processing object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "audiounit.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/param/parameterselect.h"

namespace MO {


AudioUnit::AudioUnit(int maxIn, int maxOut, bool sameNum, QObject *parent) :
    Object                  (parent),
    maximumChannelsIn_      (maxIn<0? MO_MAX_AUDIO_CHANNELS : maxIn),
    maximumChannelsOut_     (maxOut<0? MO_MAX_AUDIO_CHANNELS : maxOut),
    numberChannelsIn_       (0),
    numberChannelsOut_      (0),
    sameNumberInputOutputChannels_(sameNum)
{
    MO_DEBUG_AUDIO("AudioUnit::AudioUnit("
                   << maximumChannelsIn_ << ", " << maximumChannelsOut_
                   << ", " << parent << ")");
}

void AudioUnit::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("au", 1);
}

void AudioUnit::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("au", 1);
}

void AudioUnit::createParameters()
{
    processModeParameter_ = createSelectParameter(
                        "processmode", tr("processing"),
                        tr("Sets the processing mode"),
                        { "on", "off", "bypass" },
                        { tr("on"), tr("off"), tr("bypass") },
                        { tr("Processing is always on"),
                          tr("Processing is off, no signals are passed through"),
                          tr("The unit does no processing and passes it's input data unchanged") },
                        { PM_ON, PM_OFF, PM_BYPASS },
                        PM_ON,
                        true, false);
}

void AudioUnit::setNumberThreads(uint num)
{
    Object::setNumberThreads(num);

    audioOutputBuffer_.resize(num);
}

AudioUnit::ProcessMode AudioUnit::processMode() const
{
    MO_ASSERT(processModeParameter_, "AudioUnit::processMode without parameter");
    return processModeParameter_?
                (ProcessMode)processModeParameter_->baseValue()
              : PM_ON;
}

void AudioUnit::setNumChannelsIn(uint num, uint thread)
{
    MO_DEBUG_AUDIO("AudioUnit::setNumChannelsIn("
                   << num << ", " << thread << ")");

    numberChannelsIn_ = num;
    if (sameNumberInputOutputChannels_)
        numberChannelsOut_ = num;

    channelsChanged(thread);
}

void AudioUnit::setNumChannelsOut(uint num, uint thread)
{
    MO_DEBUG_AUDIO("AudioUnit::setNumChannelsOut("
                   << num << ", " << thread << ")");

    numberChannelsOut_ = num;
    if (sameNumberInputOutputChannels_)
        numberChannelsIn_ = num;

    channelsChanged(thread);
}

void AudioUnit::setNumChannelsInOut(uint numIn, uint numOut, uint thread)
{
    MO_DEBUG_AUDIO("AudioUnit::setNumChannelsInOut("
                   << numIn << ", " << numOut << ", " << thread << ")");

    numberChannelsIn_ = numIn;
    numberChannelsOut_ = sameNumberInputOutputChannels_? numIn : numOut;

    channelsChanged(thread);
}

void AudioUnit::channelsChanged(uint thread)
{
    audioOutputBuffer_[thread].resize(numberChannelsOut_ * bufferSize(thread));
}

void AudioUnit::processAudioBlock_(const F32 *input, Double time, uint thread)
{
    const auto mode = processMode();

    if (mode == PM_OFF)
        return;

    if (mode == PM_BYPASS)
    {
        performBypass_(input, outputBuffer(thread), thread);
    }
    else
        processAudioBlock(input, time, thread);
}


void AudioUnit::performBypass_(const F32 *input, F32 *output, uint thread) const
{
    if (numChannelsOut() == 0)
        return;

    const uint bsize = bufferSize(thread) * sizeof(F32);

    if (numChannelsIn() <= numChannelsOut())
    {
        // copy input data
        memcpy(output, input, numChannelsIn() * bsize);

        // clear remaining data
        if (numChannelsIn() < numChannelsOut())
            memset(output + numChannelsIn() * bsize,
                   0, (numChannelsOut() - numChannelsIn()) * bsize
                   );
    }
    else
    {
        // copy only so much input data
        memcpy(output, input, numChannelsOut() * bsize);
    }
}

} // namespace MO
