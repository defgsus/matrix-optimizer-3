/** @file audiodevice.cpp

    @brief Wrapper around audio portaudio api

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#include <chrono>
#include <thread>

#include <portaudio.h>

#include <QMessageBox>
#include <QThread>

#include "AudioDevice.h"
#include "AudioDevices.h"
#include "io/log_audio.h"
#include "io/error.h"
#include "io/Settings.h"


#define MO_CHECKPA(command__, text__) \
    { PaError err__ = (command__); if (err__ != paNoError) \
        MO_AUDIO_ERROR(API, text__ << " (" << Pa_GetErrorText(err__) << ")"); }

namespace MO {
namespace AUDIO {

namespace {

/** A fake thread for nil audio processing */
class FakeAudioThread : public QThread
{
public:
    FakeAudioThread()
        : doStop_   (false)
    {
    }

    ~FakeAudioThread() { if (isRunning()) stop(); }

    void setCallback(AudioDevice::Callback cb)
    {
        callback_ = cb;
    }

    void setConfiguration(const Configuration& c)
    {
        if (isRunning())
            return;

        config_ = c;
        in_.resize(config_.numChannelsIn() * config_.bufferSize());
        out_.resize(config_.numChannelsOut() * config_.bufferSize());
        period_ = std::chrono::microseconds(
                    (int64_t)(config_.sampleRateInv() * config_.bufferSize()) * 1000000
                    );
    }

    void stop() { doStop_ = true; wait(); }

    void run() Q_DECL_OVERRIDE
    {
        AudioDevice::StreamTime dummy;
        doStop_ = false;
        while (!doStop_)
        {
            if (callback_)
                callback_(&in_[0], &out_[0], dummy);

            std::this_thread::sleep_for(period_);
        }
    }

private:

    std::chrono::high_resolution_clock clock_;
    AudioDevice::Callback callback_;
    Configuration config_;
    std::chrono::duration<double> period_;
    std::vector<float> in_, out_;
    bool doStop_;
};

} // namespace







// ------------ private 3rd-api-and-such specific data ---------

class AudioDevice::Private
{
public:
    Private() : stream(0), fakeThread(0) { }

    PaStreamParameters inputParam, outputParam;
    PaStreamFlags streamFlags;
    PaStream *stream;

    FakeAudioThread * fakeThread;
};

// ---------------- callback ----------------------------

// wrapper for the Pa callback to get private
// access to AudioDevice.
// We could register the Pa callback as a friend
// but that would require <portaudio.h> in the header file
inline int mo_audio_callback_(AudioDevice * dev,
                              const AudioDevice::StreamTime& st,
                              const void * in, void * out)
{
    return dev->callback_(st, in, out);
}

// cannonical portaudio callback
static int mo_pa_callback(
        const void    * inputBuffer,
        void          * outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags /*statusFlags*/,
        void * userData )
{
    auto dev = static_cast<AudioDevice*>(userData);

    MO_ASSERT(dev, "userdata missing in audio-callback");
    MO_ASSERT(framesPerBuffer == dev->bufferSize(), "buffer size mismatch from audio-callback");
    Q_UNUSED(framesPerBuffer);

    Double st = dev->streamTime();

    return mo_audio_callback_(
              dev,
              AudioDevice::StreamTime(timeInfo->inputBufferAdcTime - st,
                                      timeInfo->currentTime - st,
                                      timeInfo->outputBufferDacTime - st),
              inputBuffer, outputBuffer);
}


int AudioDevice::callback_(const StreamTime& st, const void * in, void * out)
{
    // call user callback
    if (func_)
        func_(static_cast<const float*>(in),
              static_cast<float*>(out),
              st);

    return paContinue;
}


// ---------------- ctor -------------

AudioDevice::AudioDevice()
    :   inDeviceId_  (0),
        outDeviceId_ (0),
        ok_          (false),
        play_        (false),
        func_        (0),
        p_           (new Private)
{
    MO_DEBUG_AUDIO("AudioDevice::AudioDevice()");
}

AudioDevice::~AudioDevice()
{
    MO_DEBUG_AUDIO("AudioDevice::~AudioDevice()");

    if (ok_)
        close();

    delete p_;
}



// ---------- info -------------------

Double AudioDevice::streamTime() const
{
    return Pa_GetStreamTime(p_->stream);
}

// --------- initialisation ----------

void AudioDevice::init(int inDeviceIndex, int outDeviceIndex, const Configuration& props)
{
    init(inDeviceIndex,
         outDeviceIndex,
         props.numChannelsIn(),
         props.numChannelsOut(),
         props.sampleRate(),
         props.bufferSize());
}

void AudioDevice::initFake(uint numInputChannels,
                           uint numOutputChannels,
                           uint sampleRate,
                           uint bufferSize)
{
    MO_DEBUG("AudioDevice::init("
             << numInputChannels << ", " << numOutputChannels
             << ", " << sampleRate << ", " << bufferSize << ")");

    // close previous session
    if (ok_)
        close();

    if (!p_->fakeThread)
    {
        p_->fakeThread = new FakeAudioThread();
        p_->fakeThread->setCallback(
                    [this](const float * inp, float * outp, const StreamTime& st)
        {
            if (func_)
                func_(inp, outp, st);
        });
    }

    if (p_->fakeThread->isRunning())
        p_->fakeThread->stop();

    p_->fakeThread->setConfiguration(Configuration(
                                         numInputChannels, numOutputChannels,
                                         sampleRate, bufferSize));

    //p_->fakeThread->start(QThread::HighestPriority);

    name_ = "null";
    outName_ = "null";
    //inDeviceId_ = inDeviceIndex;
    //outDeviceId_ = outDeviceIndex;
    conf_.setNumChannelsIn(numInputChannels);
    conf_.setNumChannelsOut(numOutputChannels);
    conf_.setSampleRate(sampleRate);
    conf_.setBufferSize(bufferSize);

    MO_DEBUG_AUDIO("fake audio ok.");
    ok_ = true;
}

void AudioDevice::init(int inDeviceIndex,
                       int outDeviceIndex,
                       uint numInputChannels,
                       uint numOutputChannels,
                       uint sampleRate,
                       uint bufferSize)
{
    MO_DEBUG("AudioDevice::init(" << inDeviceIndex << ", " << outDeviceIndex
             << ", " << numInputChannels << ", " << numOutputChannels
             << ", " << sampleRate << ", " << bufferSize << ")");

    // init portaudio if not already done
    if (!AudioDevices::pa_initialized_)
    {
        MO_CHECKPA(Pa_Initialize(), "could not initialize audio library");
        AudioDevices::pa_initialized_ = true;
    }

    if (inDeviceIndex >= Pa_GetDeviceCount() || outDeviceIndex >= Pa_GetDeviceCount())
    {
        initFake(numInputChannels, numOutputChannels, sampleRate, bufferSize);
        return;
    }

    // close previous session
    if (ok_) close();

    // get device infos
    const PaDeviceInfo * ipainf = 0, * opainf = 0;

    if (inDeviceIndex >= 0)
    {
        ipainf = Pa_GetDeviceInfo(inDeviceIndex);
        if (!ipainf && numInputChannels > 0)
        {
            MO_AUDIO_ERROR(API, "can not get audio device info for input device " << inDeviceId_);
        }
    }
    else if (outDeviceIndex >= 0 && numInputChannels > 0)
    {
        ipainf = Pa_GetDeviceInfo(outDeviceIndex);
        if (!ipainf && numInputChannels > 0)
        {
            MO_AUDIO_ERROR(API, "can not get audio device info for input/output device " << inDeviceId_);
        }
    }

    if (outDeviceIndex >= 0)
    {
        opainf = Pa_GetDeviceInfo(outDeviceIndex);
        if (!opainf && numOutputChannels > 0)
        {
            MO_AUDIO_ERROR(API, "can not get audio device info for output device " << outDeviceId_);
        }
    }

    // setup in/out params

    PaStreamParameters * pap_in = 0;
    if (ipainf)
    {
        p_->inputParam.device = inDeviceIndex >= 0 ? inDeviceIndex : outDeviceIndex;
        p_->inputParam.channelCount = numInputChannels;
        p_->inputParam.sampleFormat = paFloat32;
        p_->inputParam.suggestedLatency = ipainf->defaultLowInputLatency;
        p_->inputParam.hostApiSpecificStreamInfo = 0;
        pap_in = &p_->inputParam;
    }

    PaStreamParameters * pap_out = 0;
    if (outDeviceIndex >= 0)
    {
        p_->outputParam.device = outDeviceIndex;
        p_->outputParam.channelCount = numOutputChannels;
        p_->outputParam.sampleFormat = paFloat32;
        p_->outputParam.suggestedLatency = opainf->defaultLowOutputLatency;
        p_->outputParam.hostApiSpecificStreamInfo = 0;
        pap_out = &p_->outputParam;
    }

    // ---- check default parameters ---

    if (sampleRate == 0) {
        sampleRate = opainf ? opainf->defaultSampleRate :
                              ipainf ? ipainf->defaultSampleRate : 44100;
    }

    if (bufferSize == 0)
        bufferSize = p_->outputParam.suggestedLatency * sampleRate;

    // --- stream flags ----

    p_->streamFlags = 0;

    // ---- open stream ----

    MO_PRINT("opening audio stream"
                   << "\nindevice   " << inDeviceIndex << " " << (ipainf ? ipainf->name : "-")
                   << "\noutdevice  " << outDeviceIndex << " " << (opainf ? opainf->name : "-")
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
        "could not init audiodevice '"
                << (ipainf ? ipainf->name : "-") << "'/'"
                << (opainf ? opainf->name : "-") << "'");

    // store parameters

    name_ = ipainf ? ipainf->name : "-";
    outName_ = opainf ? opainf->name : "-";
    inDeviceId_ = inDeviceIndex;
    outDeviceId_ = outDeviceIndex;
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

    if (p_->stream)
    {
        // XXX this error check is somewhat strange
        // but if we can't close the device,
        // we break here and leave the ok() flag on
        MO_CHECKPA(
            Pa_CloseStream(p_->stream),
            "could not close audio device '" << name_ << "'");
        p_->stream = 0;
    }

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

    if (p_->stream)
        MO_CHECKPA(Pa_StartStream(p_->stream), "could not start audio stream")
    else
        if (p_->fakeThread)
            p_->fakeThread->start(QThread::HighestPriority);

    play_ = true;
}

void AudioDevice::stop()
{
    MO_DEBUG_AUDIO("AudioDevice::stop()");

    if (!(ok_ && play_)) return;

    if (p_->fakeThread)
        p_->fakeThread->stop();

    if (p_->stream)
        MO_CHECKPA(Pa_StopStream(p_->stream), "could not stop audio stream");

    play_ = false;
}

bool AudioDevice::isAudioConfigured()
{
    const QString inname = settings()->getValue("Audio/indevice").toString();
    const QString outname = settings()->getValue("Audio/outdevice").toString();
    // it's enough to have outputs defined
    return !(outname.isEmpty() || outname == "None");
}

Configuration AudioDevice::defaultConfiguration()
{
    return Configuration(
            settings()->value("Audio/samplerate", 44100).toInt(),
            settings()->value("Audio/buffersize", 256).toInt(),
            settings()->value("Audio/channelsIn", 0).toInt(),
            settings()->value("Audio/channelsOut", 0).toInt());
}

bool AudioDevice::initFromSettings()
{
    MO_DEBUG_AUDIO("AudioDevice::initFromSettings()");

    // check config

    QString inDeviceName = settings()->getValue("Audio/indevice").toString();

    QString outDeviceName = settings()->getValue("Audio/outdevice").toString();
    if (outDeviceName.isEmpty())
        return false;

    // get device list

    AudioDevices devs;
    try
    {
        devs.checkDevices();
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(0, QMessageBox::tr("Error"),
                             QMessageBox::tr("Failed to initialize audio devices.\n%1").arg(e.what())
                             );
        return false;
    }

    // find configured device

    int inidx = -1;
    for (uint i=0; i<devs.numDevices(); ++i)
    {
        if (devs.getDeviceInfo(i)->name == inDeviceName)
        {
            inidx = i;
            break;
        }
    }
    int outidx = -1;
    for (uint i=0; i<devs.numDevices(); ++i)
    {
        if (devs.getDeviceInfo(i)->name == outDeviceName)
        {
            outidx = i;
            break;
        }
    }

    if (inidx < 0 && !(inDeviceName.isEmpty() || inDeviceName == "None"))
    {
        QMessageBox::warning(0, QMessageBox::tr("Error"),
                             QMessageBox::tr("The configured audio input device '%1' could not be found.")
                             .arg(inDeviceName)
                             );
        return false;
    }
    if (outidx < 0)
    {
        QMessageBox::warning(0, QMessageBox::tr("Error"),
                             QMessageBox::tr("The configured audio output device '%1' could not be found.")
                             .arg(outDeviceName)
                             );
        return false;
    }

    // get the other settings
    uint sampleRate = settings()->getValue("Audio/samplerate").toInt();
    uint bufferSize = settings()->getValue("Audio/buffersize").toInt();
    uint numInputs = settings()->getValue("Audio/channelsIn").toInt();
    uint numOutputs = settings()->getValue("Audio/channelsOut").toInt();

    // initialize

    try
    {
        init(inidx, outidx, numInputs, numOutputs, sampleRate, bufferSize);
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(0, QMessageBox::tr("Error"),
                             QMessageBox::tr("Failed to init audio device '%1' or '%2'.\n%3")
                             .arg(inDeviceName)
                             .arg(outDeviceName)
                             .arg(e.what())
                             );
        return false;
    }

    return true;
}




#undef MO_CHECKPA

} // namespace AUDIO
} // namespace MO
