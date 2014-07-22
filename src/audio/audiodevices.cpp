/** @file audiodevices.cpp

    @brief Information center and initialiser for audio devices.

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#include <portaudio.h>

#include "audiodevices.h"
#include "io/log.h"
#include "io/error.h"
#include "audiodevice.h"


namespace MO {
namespace AUDIO {


#define MO_CHECKPA(command__, text__) \
    { PaError err__ = (command__); if (err__ != paNoError) \
        MO_AUDIO_ERROR(API, text__ << " (" << Pa_GetErrorText(err__) << ")"); }


bool AudioDevices::pa_initialized_ = false;


AudioDevices::AudioDevices()
    :   numDevices_ (0),
        numApis_    (0)
{
    MO_DEBUG_AUDIO("AudioDevices::AudioDevices()");
}

AudioDevices::~AudioDevices()
{
    MO_DEBUG_AUDIO("AudioDevices::~AudioDevices()");
}


// ---------- info -------------------

bool AudioDevices::checkDevices()
{
    MO_DEBUG_AUDIO("AudioDevices::checkDevices()");

    if (!pa_initialized_)
    {
        MO_CHECKPA(Pa_Initialize(), "could not initialize audio library");
        pa_initialized_ = true;
    }

    api_infos_.clear();
    dev_infos_.clear();

    // ----------- apis ------------------

    numApis_ = Pa_GetHostApiCount();

    for (uint i=0; i<numApis_; ++i)
    {
        auto pa = Pa_GetHostApiInfo(i);

        ApiInfo inf;
        inf.name = pa->name;
        inf.numDevices = pa->deviceCount;

        api_infos_.push_back(inf);
    }

    // ----------- devices ---------------

    numDevices_ = Pa_GetDeviceCount();

    for (uint i=0; i<numDevices_; ++i)
    {
        auto pa = Pa_GetDeviceInfo(i);

        DeviceInfo inf;
        inf.name = pa->name;
        inf.numInputChannels = pa->maxInputChannels;
        inf.numOutputChannels = pa->maxOutputChannels;
        inf.defaultSampleRate = pa->defaultSampleRate;
        inf.defaultBufferLength = pa->defaultLowOutputLatency
                                    * pa->defaultSampleRate;
        // sometimes Pa returns bogus values
        if (inf.defaultBufferLength > 4096)
            inf.defaultBufferLength = 4096;

        dev_infos_.push_back(inf);
    }

    return numDevices_ != 0;
}

size_t AudioDevices::numDevices() const
{
    return numDevices_;
}

const AudioDevices::DeviceInfo * AudioDevices::getDeviceInfo(size_t index) const
{
    if (index < dev_infos_.size())
        return &dev_infos_[index];

    return 0;
}

size_t AudioDevices::numApis() const
{
    return numApis_;
}

const AudioDevices::ApiInfo * AudioDevices::getApiInfo(size_t index) const
{
    if (index < api_infos_.size())
        return &api_infos_[index];

    return 0;
}

void AudioDevices::dump_info(std::ostream &out) const
{
    out << "audio APIs:\n";
    for (size_t i=0; i<numApis_; ++i)
    {
        out << "index    " << i << "\n"
            << "name     " << api_infos_[i].name << "\n"
            << "devices  " << api_infos_[i].numDevices << "\n\n";
    }

    out << "\naudio devices:\n";

    for (size_t i=0; i<numDevices_; ++i)
    {
        out << "index    " << i << "\n"
            << "name     " << dev_infos_[i].name << "\n"
            << "channels " << dev_infos_[i].numInputChannels
                    << " / " << dev_infos_[i].numOutputChannels << "\n\n";
    }

    out << std::endl;
}

#undef MO_CHECKPA



} // namespace AUDIO
} // namespace MO

