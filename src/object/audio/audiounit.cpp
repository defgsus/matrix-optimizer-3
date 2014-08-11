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
#include "object/scene.h"

namespace MO {


AudioUnit::AudioUnit(int maxIn, int maxOut, bool sameNum, QObject *parent) :
    Object                  (parent),
    maximumChannelsIn_      ( maxIn<0? MO_MAX_AUDIO_CHANNELS : maxIn),
    maximumChannelsOut_     (maxOut<0? MO_MAX_AUDIO_CHANNELS : maxOut),
    numberChannelsIn_       ( maxIn==0? 0 : 1),
    numberChannelsOut_      (maxOut==0? 0 : 1),
    numberRequestedChannelsOut_ (numberChannelsOut_),
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
    Object::createParameters();

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

void AudioUnit::setBufferSize(uint bufferSize, uint thread)
{
    Object::setBufferSize(bufferSize, thread);

    resizeAudioOutputBuffer_();
}

AudioUnit::ProcessMode AudioUnit::processMode() const
{
    MO_ASSERT(processModeParameter_,
              "AudioUnit::processMode() queried but no parameter created yet");

    return processModeParameter_?
                (ProcessMode)processModeParameter_->baseValue()
              : PM_ON;
}

void AudioUnit::setNumChannelsIn(uint num)
{
    MO_DEBUG_AUDIO("AudioUnit('" << idName() << "')::setNumChannelsIn("
                   << num << ") "
                   "requestedOut_ == " << numberRequestedChannelsOut_);

    const uint oldin = numberChannelsIn_,
               oldout = numberChannelsOut_;

    numberChannelsIn_ = std::min(maximumChannelsIn_, num);
    if (sameNumberInputOutputChannels_)
        numberChannelsOut_ = std::min(maximumChannelsOut_, num);

    numberRequestedChannelsOut_ = numberChannelsOut_;

    if (oldin != numberChannelsIn_ || oldout != numberChannelsOut_)
        channelsChanged();
}

void AudioUnit::setNumChannelsOut(uint num)
{
    MO_DEBUG_AUDIO("AudioUnit('" << idName() << "')::setNumChannelsOut("
                   << num << ") "
                   "requestedOut_ == " << numberRequestedChannelsOut_);

    const uint oldin = numberChannelsIn_,
               oldout = numberChannelsOut_;

    numberChannelsOut_ = std::min(maximumChannelsOut_, num);
    if (sameNumberInputOutputChannels_)
        numberChannelsIn_ = std::min(maximumChannelsIn_, num);

    numberRequestedChannelsOut_ = numberChannelsOut_;

    if (oldin != numberChannelsIn_ || oldout != numberChannelsOut_)
        channelsChanged();
}

void AudioUnit::setNumChannelsInOut(uint numIn, uint numOut)
{
    MO_DEBUG_AUDIO("AudioUnit('" << idName() << "')::setNumChannelsInOut("
                   << numIn << ", " << numOut << ") "
                   "requestedOut_ == " << numberRequestedChannelsOut_);

    const uint oldin = numberChannelsIn_,
               oldout = numberChannelsOut_;

    numberChannelsIn_ = std::min(maximumChannelsIn_, numIn);
    numberChannelsOut_ = std::min(maximumChannelsOut_,
                sameNumberInputOutputChannels_? numIn : numOut);

    numberRequestedChannelsOut_ = numberChannelsOut_;

    if (oldin != numberChannelsIn_ || oldout != numberChannelsOut_)
        channelsChanged();
}

void AudioUnit::channelsChanged()
{
    resizeAudioOutputBuffer_();
}


void AudioUnit::resizeAudioOutputBuffer_()
{
    MO_DEBUG_AUDIO("AudioUnit('" << idName() << "')::resizeAudioOutputBuffer_() "
                   "in/out == " << numberChannelsIn_ << "/" << numberChannelsOut_);

    for (uint i=0; i<audioOutputBuffer_.size(); ++i)
    {
        audioOutputBuffer_[i].resize(numberChannelsOut_ * bufferSize(i));
        MO_DEBUG_AUDIO("AudioUnit('" << idName() << "')::audioOutputBuffer_["
                       << i << "].size() == " << audioOutputBuffer_[i].size()
                       << ", bufferSize(" << i << ") == " << bufferSize(i));
    }
}

void AudioUnit::childrenChanged()
{
    Object::childrenChanged();

    // always keep track of direct sub-units
    subAudioUnits_ = findChildObjects<AudioUnit>();
}

void AudioUnit::updateSubUnitChannels()
{
    // break if this is no pass-through unit
    if (numChannelsOut() < 1)
        return;

    // update channel sizes of sub-units
    for (auto au : subAudioUnits_)
    {
        if (au->numChannelsIn() != numChannelsOut()
            || au->numRequestedChannelsOut() != au->numChannelsOut())
            au->setNumChannelsInOut(numChannelsOut(),
                                    au->numRequestedChannelsOut());

        au->updateSubUnitChannels();
    }
}

void AudioUnit::requestNumChannelsOut_(uint newNumber)
{
    numberRequestedChannelsOut_ = newNumber;

    /*
    Scene * scene = sceneObject();
    if (!scene)
    {
        setNumChannelsOut(newNumber);
        updateSubUnitChannels();
    }
    else
    {
        numberRequestedChannelsOut_ = newNumber;
        scene->updateAudioUnitChannels();
    }*/
}







void AudioUnit::processAudioBlock_(const F32 *input, Double time, uint thread, bool recursive)
{
    const auto mode = processMode();

    if (mode == PM_OFF)
        return;

    MO_ASSERT(audioOutputBuffer_[thread].size() >=
              numChannelsOut() * bufferSize(thread),
              "AudioUnit::processAudioBlock_() audioOutputBuffer_ too small, "
              "is " << audioOutputBuffer_[thread].size() << ", should be "
              << (numChannelsOut() * bufferSize(thread)));

    if (mode == PM_BYPASS)
    {
        performBypass_(input, outputBuffer(thread), thread);
    }
    else
        processAudioBlock(input, time, thread);

    // break if this is no pass-through unit
    if (recursive && numChannelsOut() > 0)
        for (auto au : subAudioUnits_)
            au->processAudioBlock_(outputBuffer(thread), time, thread, recursive);
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
