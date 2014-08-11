/** @file audiodevice.cpp

    @brief Wrapper around audio portaudio api

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#include <portaudio.h>

#include <QMessageBox>

#include "audiodevice.h"
#include "audiodevices.h"
#include "io/log.h"
#include "io/error.h"
#include "io/settings.h"


#define MO_CHECKPA(command__, text__) \
    { PaError err__ = (command__); if (err__ != paNoError) \
        MO_AUDIO_ERROR(API, text__ << " (" << Pa_GetErrorText(err__) << ")"); }

namespace MO {
namespace AUDIO {


// ------------ private portaudio specific data ---------

class AudioDevice::Private
{
public:
    PaStreamParameters inputParam, outputParam;
    PaStreamFlags streamFlags;
    PaStream *stream;
};

// ---------------- callback ----------------------------

// wrapper for the Pa callback to get private
// access to AudioDevice.
// We could register the Pa callback as a friend
// but that would require <portaudio.h> in the header file
inline int mo_audio_callback_(AudioDevice * dev, const void * in, void * out)
{
    return dev->callback_(in, out);
}

// cannonical portaudio callback
static int mo_pa_callback(
        const void    * inputBuffer,
        void          * outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* /*timeInfo*/,
        PaStreamCallbackFlags /*statusFlags*/,
        void * userData )
{
    auto dev = static_cast<AudioDevice*>(userData);

    MO_ASSERT(dev, "userdata missing in audio-callback");
    MO_ASSERT(framesPerBuffer == dev->bufferSize(), "buffer size mismatch from audio-callback");
    Q_UNUSED(framesPerBuffer);

    return mo_audio_callback_(dev, inputBuffer, outputBuffer);
}


int AudioDevice::callback_(const void * in, void * out)
{
    // call user callback
    if (func_)
        func_(static_cast<const float*>(in), static_cast<float*>(out));

    return paContinue;
}




// ---------------- ctor -------------

AudioDevice::AudioDevice()
    :   deviceId_   (0),
        ok_         (0),
        play_       (0),
        func_       (0),
        p_          (new Private)
{
    MO_DEBUG_AUDIO("AudioDevice::AudioDevice()");
}

AudioDevice::~AudioDevice()
{
    MO_DEBUG_AUDIO("AudioDevice::~AudioDevice()");

    if (ok_) close();

    delete p_;
}



// ---------- info -------------------


// --------- initialisation ----------

void AudioDevice::init(uint deviceIndex, const Configuration& props)
{
    init(deviceIndex,
         props.numChannelsIn(),
         props.numChannelsOut(),
         props.sampleRate(),
         props.bufferSize());
}

void AudioDevice::init(uint deviceIndex,
                       uint numInputChannels,
                       uint numOutputChannels,
                       uint sampleRate,
                       uint bufferSize)
{
    MO_DEBUG_AUDIO("AudioDevice::init(" << deviceIndex << ", " << numInputChannels <<
             ", " << numOutputChannels << ", " << sampleRate << ", " << bufferSize << ")");

    // init portaudio if not already done
    if (!AudioDevices::pa_initialized_)
    {
        MO_CHECKPA(Pa_Initialize(), "could not initialize audio library");
        AudioDevices::pa_initialized_ = true;
    }

    // close previous session
    if (ok_) close();

    // get some info of device
    auto painf = Pa_GetDeviceInfo(deviceIndex);
    if (!painf)
    {
        MO_AUDIO_ERROR(API, "can not get audio device info for device " << deviceId_);
    }

    // setup in/out params

    PaStreamParameters * pap_in = 0;
    if (numInputChannels)
    {
        p_->inputParam.device = deviceIndex;
        p_->inputParam.channelCount = numOutputChannels;
        p_->inputParam.sampleFormat = paFloat32;
        p_->inputParam.suggestedLatency = painf->defaultLowInputLatency;
        p_->inputParam.hostApiSpecificStreamInfo = 0;
        pap_in = &p_->inputParam;
    }

    PaStreamParameters * pap_out = 0;
    if (numOutputChannels)
    {
        p_->outputParam.device = deviceIndex;
        p_->outputParam.channelCount = numOutputChannels;
        p_->outputParam.sampleFormat = paFloat32;
        p_->outputParam.suggestedLatency = painf->defaultLowOutputLatency;
        p_->outputParam.hostApiSpecificStreamInfo = 0;
        pap_out = &p_->outputParam;
    }

    // ---- check default parameters ---

    if (sampleRate == 0)
        sampleRate = painf->defaultSampleRate;

    if (bufferSize == 0)
        bufferSize = p_->outputParam.suggestedLatency * sampleRate;

    // --- stream flags ----

    p_->streamFlags = 0;

    // ---- open stream ----

    MO_DEBUG("opening audio stream"
                   << "\ndevice     " << deviceIndex << " " << painf->name
                   << "\nsamplerate " << sampleRate
                   << "\nbuffersize " << bufferSize
                   << "\nchannels   " << numInputChannels << " / " << numOutputChannels
                   );

    MO_CHECKPA(
    Pa_OpenStream(&p_->stream,
                  pap_in,
                  pap_out,
                  sampleRate,
                  bufferSize,
                  p_->streamFlags,
                  mo_pa_callback,
                  static_cast<void*>(this)
                  ),
        "could not init audiodevice '" << painf->name << "'");

    // store parameters

    name_ = painf->name;
    deviceId_ = deviceIndex;
    conf_.setNumChannelsIn(numInputChannels);
    conf_.setNumChannelsOut(numOutputChannels);
    conf_.setSampleRate(sampleRate);
    conf_.setBufferSize(bufferSize);

    MO_DEBUG_AUDIO("audio ok.");
    ok_ = true;
}


void AudioDevice::close()
{
    MO_DEBUG_AUDIO("AudioDevice::close()");

    // no error reporting nescessary, i think
    if (!ok_) return;

    if (play_)
        stop();

    // XXX this error check is somewhat strange
    // but if we can't close the device,
    // we break here and leave the ok() flag on
    MO_CHECKPA(
        Pa_CloseStream(p_->stream),
        "could not close audio device '" << name_ << "'");

    // reset params
    ok_ = play_ = false;
}


void AudioDevice::setCallback(Callback func)
{
    MO_DEBUG_AUDIO("AudioDevice::setCallback(...)");

    func_ = func;
}


// ------------- runtime -------------

void AudioDevice::start()
{
    MO_DEBUG_AUDIO("AudioDevice::start()");

    if (!ok_) return;

    MO_CHECKPA(Pa_StartStream(p_->stream), "could not start audio stream");

    play_ = true;
}

void AudioDevice::stop()
{
    MO_DEBUG_AUDIO("AudioDevice::stop()");

    if (!(ok_ && play_)) return;

    MO_CHECKPA(Pa_StopStream(p_->stream), "could not stop audio stream");

    play_ = false;
}

bool AudioDevice::isAudioConfigured() const
{
    const QString name = settings->getValue("Audio/device").toString();
    return !(name.isEmpty() || name == "None");
}

uint AudioDevice::numConfiguredInputChannels() const
{
    return settings->value("Audio/channelsIn", 0).toInt();
}

bool AudioDevice::initFromSettings()
{
    MO_DEBUG_AUDIO("AudioDevice::initFromSettings()");

    // check config

    QString deviceName = settings->getValue("Audio/device").toString();
    if (deviceName.isEmpty())
        return false;

    // get device list

    AudioDevices devs;
    try
    {
        devs.checkDevices();
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(0, QWidget::tr("Error"),
                             QWidget::tr("Failed to initialize audio devices.\n%1").arg(e.what())
                             );
        return false;
    }

    // find configured device

    int idx = -1;
    for (uint i=0; i<devs.numDevices(); ++i)
    {
        if (devs.getDeviceInfo(i)->name == deviceName)
        {
            idx = i;
            break;
        }
    }

    if (idx < 0)
    {
        QMessageBox::warning(0, QWidget::tr("Error"),
                             QWidget::tr("The configured device '%1' could not be found.")
                             .arg(deviceName)
                             );
        return false;
    }

    // get the other settings
    uint sampleRate = settings->getValue("Audio/samplerate").toInt();
    uint bufferSize = settings->getValue("Audio/buffersize").toInt();
    uint numInputs = settings->getValue("Audio/channelsIn").toInt();
    uint numOutputs = settings->getValue("Audio/channelsOut").toInt();

    // initialize

    try
    {
        init(idx, numInputs, numOutputs, sampleRate, bufferSize);
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(0, QWidget::tr("Error"),
                             QWidget::tr("Failed to init audio device '%1'.\n%2")
                             .arg(deviceName)
                             .arg(e.what())
                             );
        return false;
    }

    return true;
}




#undef MO_CHECKPA

} // namespace AUDIO
} // namespace MO
