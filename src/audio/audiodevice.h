/** @file audiodevice.h

    @brief Wrapper around audio portaudio api

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#ifndef MOSRC_AUDIO_AUDIODEVICE_H
#define MOSRC_AUDIO_AUDIODEVICE_H

#include <functional>

#include <QString>

#include "audio/configuration.h"


namespace MO {
namespace AUDIO {

/** @brief Wrapper around audio api

    Currently simply wraps portaudio.

    @code
    // example
    #include "audio/audiodevices.h"
    #include "audio/audiodevice.h"
    #include <functional>
    #include <math.h>

    void callback(const float * in, float * out,
                  uint rate, uint chan, uint length)
    {
        // sample count
        static long int sam = 0;
        for (uint i=0; i<length; ++i)
        {
            // ~200 hz tone
            float s = sinf( (float)sam++ * 6.28f * 200.f / rate );

            // fill each channel
            for (uint j=0; j<chan; ++j)
                *out++ = s;
        }
    }

    void main()
    {
        using namespace CSMOD;
        using namespace std::placeholders;

        AudioDevices devs;
        devs.checkDevices();
        devs.dump_info();

        AudioDevice dev;
        // 0-device is commonly working
        dev.init(0, 2,2, 44100, 512);
        // bind some arguments to the callback
        dev.setCallback(std::bind(some, _1, _2,
                dev.sampleRate(), dev.numOutputChannels(), dev.bufferSize()));

        dev.start();

        sleep(1);

        dev.stop();
        dev.close();
    }

    @endcode
*/
class AudioDevice
{
    // access from the callback to this class
    friend int mo_audio_callback_(AudioDevice*, const void*, void*);

public:

    // ------------ types ----------------

    /** The type of function issued by this class for
        audio callback. */
    typedef std::function<void(const F32*, F32*)> Callback;

    // --------------- ctor --------------

    AudioDevice();

    /** This will, if nescessary, stop and close the stream. */
    ~AudioDevice();

    // ---------- info -------------------

    /** Returns true when an audio stream is initialized */
    bool isOk() const { return ok_; }
    /** Returns true when the audio stream is running/playing */
    bool isPlaying() const { return play_; }

    /** name of the audio device */
    const QString& name() const { return name_; }
    /** host-os index of the selected device */
    uint inDeviceId() const { return inDeviceId_; }
    uint outDeviceId() const { return outDeviceId_; }

    /** Returns the audio configuration */
    const Configuration configuration() const { return conf_; }
    /** number of channel in input */
    uint numInputChannels() const { return conf_.numChannelsIn(); }
    /** number of channels to output */
    uint numOutputChannels() const { return conf_.numChannelsOut(); }
    /** samplerate in hertz (samples per second) */
    uint sampleRate() const { return conf_.sampleRate(); }
    /** length of buffer in samples. */
    uint bufferSize() const { return conf_.bufferSize(); }

    // --------- initialisation ----------

    /** Returns true when audio-settings are already made. */
    static bool isAudioConfigured();

    /** Returns the configuration as set by user previously */
    static Configuration defaultConfiguration();

    /** Initializes a device/stream from the stored settings.
        Returns true when this worked.
        Returns false otherwise or if no settings are made yet.
        Shows an error dialog on api failure. */
    bool initFromSettings();

    /** Initializes a device/stream.
        @p inDeviceIndex and outDeviceIndex are the host-os device numbers as returned by CSMOD::AudioDevices.
        One of them can be set to -1, if you do not need the particular stream.
        @p numInputChannels and @p numOutputChannels is the number of channels requested
        for the input and output.
        @p sampleRate sets the desired samples per second and can be left zero
        to use the device's default.
        @p bufferLength defines the desired samples in a buffer, and therefore the
        input/output latency. It can also be left zero to select the device's default.
        Note that a bufferlength of, e.g., 128 for two output channels means, that
        you actually need to generate 256 samples.
        @throws AudioException on any error.
        */
    void init(int inDeviceIndex,
              int outDeviceIndex,
              uint numInputChannels,
              uint numOutputChannels,
              uint sampleRate = 0,
              uint bufferSize = 0);

    /** look for other init() function for description. */
    void init(int inDeviceIndex, int outDeviceIndex, const Configuration& props);

    /** close the audio stream.
        @throws AudioException on any error.
        */
    void close();

    /** Installs a function that is called from the audio device for reading/filling
        the buffer. The callback is expected to look like this:
        @code
        void callback(const float * input, float * output);
        @endcode
        Of course you can use std::bind to link other stuff here.
        input holds numInputChannels() * bufferLength() floats and output
        has place for numOutputChannels() * bufferLength() floats.
        @note It's common practice to only do memory-shifting and calculation
        in this thread and not call any OS functions. Some say you should even
        avoid mutexes. */
    void setCallback(Callback func);

    // ------------- runtime -------------

    /** Starts the audio thread once the stream is initialized with init() */
    void start();
    /** Temporarily stops the audio thread. Can be reenabled with start(). */
    void stop();

    // _________ HIDDEN AREA _____________

private:

    int callback_(const void * in, void * out);

    uint inDeviceId_, outDeviceId_;
    Configuration conf_;

    QString name_, outName_;

    volatile bool ok_, play_;

    Callback func_;

    // backend library specific data
    class Private;
    Private * p_;
};


} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_AUDIODEVICE_H
