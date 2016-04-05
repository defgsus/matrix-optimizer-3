/** @file liveaudioengine.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

/* define MO_BUFFER_TRICK
    // to switch to non-working gfx-time-between-audio-blocks solution test */

#include <tuple>
#ifdef MO_BUFFER_TRICK
#   include <atomic>
#endif

#include <QThread>
#include <QMessageBox>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "liveaudioengine.h"
#include "object/scene.h"
#include "object/util/scenelock_p.h"
#include "object/util/audioobjectconnections.h"
#include "tool/locklessqueue.h"
#include "audio/audiodevice.h"
#include "audio/configuration.h"
#include "audio/tool/audiobuffer.h"
#include "engine/audioengine.h"
#include "io/time.h"
#include "io/error.h"
#include "io/log_audio.h"

namespace MO {




// ################################## LiveAudioEngine::Private ########################################

class AudioEngineInThread;
class AudioEngineOutThread;

class LiveAudioEngine::Private
{
    friend class AudioEngineOutThread;
public:

    Private(LiveAudioEngine * e)
        : parent        (e),
#ifdef MO_BUFFER_TRICK
          curSample     (0),
#endif
          engine        (new AudioEngine()),
          isPause       (false),
          engineChanged (false),
          startTime     (0.),
          timeOffset    (0.),
          audioDevice   (0),
          defaultConf   (AUDIO::AudioDevice::defaultConfiguration()),
          audioOutThread(0)
    { }

    ~Private()
    {
        delete audioDevice;
        delete engine;
    }

    /** Recreates the AudioEngine dsp path */
    void updateScene();
    void audioCallback(const F32 *, F32 *, const AUDIO::AudioDevice::StreamTime&);

#ifdef MO_BUFFER_TRICK
    struct BufferTime
    {
        BufferTime() noexcept : sample(0), time(0.) { }
        BufferTime(SamplePos s, Double t) noexcept : sample(s), time(t) { }
        SamplePos sample;
        Double time;
    };
    std::atomic<BufferTime> lastBuffer;
    SamplePos curSample;
#endif

    LiveAudioEngine * parent;
    AudioEngine * engine;
    bool isPause, engineChanged;
    //Double lastBufferTime;
    Double startTime, timeOffset;

    AUDIO::AudioDevice * audioDevice;
    AUDIO::Configuration defaultConf;

    //QReadWriteLock engineLock;

    //AudioInThread * audioInThread;
    AudioEngineOutThread * audioOutThread;
    LocklessQueue<const F32*> audioInQueue;
    LocklessQueue<const F32*> audioOutQueue;
};








// ################################ audio out worker thread #########################################

class AudioEngineOutThread : public QThread
{
public:
    AudioEngineOutThread(LiveAudioEngine * engine, QObject * parent)
        : QThread   (parent),
          engine_   (engine),
          stop_     (false)
    {

    }

    void stop() { stop_ = true; wait(); }

    void run()
    {
        setCurrentThreadName("AUDIO_OUT");
        MO_DEBUG("AudioOutThread::run()");

        uint
            bufferSize = engine_->config().bufferSize(),
            numChannelsOut = engine_->config().numChannelsOut(),
            bufferSizeChan = bufferSize * numChannelsOut,
            numAhead = 4;

        AUDIO::AudioBuffer
                    bufferForDevice(bufferSizeChan, numAhead);

        auto live = engine_->p_;

        // length of a buffer in microseconds
        unsigned long bufferTimeU =
            1000000 * bufferSize * engine_->config().sampleRateInv();

        while (!stop_)
        {
            // calc buffers for next system-out callback
            if (live->audioOutQueue.count() < numAhead && !engine_->isPause())
            {
                const F32 * inputFromDevice;
                if (!live->audioInQueue.consume(inputFromDevice))
                    inputFromDevice = 0;

                {
                    ScopedSceneLockRead lock(engine_->scene());

                    // calculate an audio block
                    live->engine->processForDevice(
                                inputFromDevice,
                                bufferForDevice.writePointer());

                    // check for engine swap
                    if (live->engineChanged)
                    {
                        live->engineChanged = false;

                        // update local settings
                        bufferSize = engine_->config().bufferSize(),
                        numChannelsOut = engine_->config().numChannelsOut(),
                        bufferSizeChan = bufferSize * numChannelsOut,
                        bufferTimeU = 1000000 *
                                bufferSize * engine_->config().sampleRateInv();

                        bufferForDevice.setSize(bufferSizeChan, numAhead);
                        continue; //< dont send an invalid bufferForDevice
                    }

                }

                // publish
                bufferForDevice.nextBlock();
                live->audioOutQueue.produce(
                            bufferForDevice.readPointer());

            }
            else
                usleep(bufferTimeU);
        }

        live->engine->sendCloseThread();

        MO_DEBUG("AudioOutThread::run() finished");
    }

private:

    LiveAudioEngine * engine_;

    volatile bool stop_;
};
















// ################################## LiveAudioEngine implementation ########################################


LiveAudioEngine::LiveAudioEngine(QObject *parent)
    : QObject       (parent),
      p_            (new Private(this))
{
}

LiveAudioEngine::~LiveAudioEngine()
{
    stop();

    delete p_;
}

Scene * LiveAudioEngine::scene() const
{
    return p_->engine->scene();
}

uint LiveAudioEngine::thread() const
{
    return p_->engine->thread();
}

const AUDIO::Configuration& LiveAudioEngine::config() const
{
    return p_->engine->config();
}

SamplePos LiveAudioEngine::pos() const
{
    return p_->engine->pos();
}

Double LiveAudioEngine::second() const
{
    // XXX See below
    //if (!isPlayback() || isPause())
        return p_->engine->second();

    /** @todo find best solution for gfx time between audo dsp-blocks.
        Naive approach was to store the systemtime when the buffer is send
        to the audiodevice and use the buffertime + time-since for graphics.
        Unfortunately it hops back and forth that way. No clue why!
        Now we use the time since stream start and *slightly* adjust
        an offset to follow the actual audio time. Since the audio time
        is hopping (because of irregular buffer-request-times) we only follow
        *slightly*. That way the times will theoretically stay in sync for hours
        and days and weeks..
        XXX Still does not work! Time is not exact at start and continues on pause
    */

#ifdef MO_BUFFER_TRICK
    const Private::BufferTime bt = p_->lastBuffer;

    return bt.sample * config().sampleRateInv()
           + applicationTime() - bt.time;
#else

    Double audioTime = p_->engine->second(),
           time = applicationTime() - p_->startTime;
    // slowly adjust to audio time
    p_->timeOffset += 0.0001 * (audioTime - (time + p_->timeOffset));

    return time + p_->timeOffset;
#endif
}

const F32 * LiveAudioEngine::outputEnvelope() const
{
    return p_->engine->outputEnvelope();
}


void LiveAudioEngine::seek(SamplePos pos)
{
    p_->engine->seek(pos);
#ifdef MO_BUFFER_TRICK
    p_->curSample = pos;
#else
    if (isPlayback())
        p_->startTime = applicationTime() - (p_->engine->second() + p_->timeOffset);
#endif
}

void LiveAudioEngine::seek(Double time)
{
    seek( SamplePos(time * p_->engine->config().sampleRate()) );
}

void LiveAudioEngine::setScene(Scene * s, uint thread)
{
#ifdef xxx_this_does_not_work_because_the_audioobjects_are_touched_by_AudioEngine_setScene
    // dont care for threads
    if (!isPlayback())
    {
        // simply reassign
        p_->engine->setScene(s, p_->defaultConf, thread);
        return;
    }

    // create engine, blocking on caller thread
    auto eng = new AudioEngine();
    eng->setScene(s, p_->defaultConf, thread);
    // reuse current scene time
    if (p_->engine)
        eng->seek(p_->engine->pos());

    // lock access to p_->nextEngine
    QWriteLocker lock(&p_->engineLock);

    // already a request?
    if (p_->nextEngine)
        delete p_->nextEngine;

    p_->nextEngine = eng;
#else

    // dont care for threads
    if (!isPlayback())
    {
        MO_DEBUG("LiveAudioEngine::setScene(" << s << ", " << thread << ") non-playback");
        // simply reassign
        p_->engine->setScene(s, p_->defaultConf, thread);
        return;
    }

    // update the same scene?
    if (scene() && s == scene())
    {
        ScopedSceneLockWrite lock(scene());

        MO_DEBUG("LiveAudioEngine::setScene(" << s << ", " << thread << ") thread-safe swap");

        p_->engine->setScene(scene(), config(), thread);
        p_->engineChanged = true;
        /*if (!p_->nextEngine)
            p_->nextEngine = new AudioEngine();
        p_->nextEngine->setScene(scene(), config(), thread);
        delete p_->engine;*/
    }
    else
    {
        MO_ASSERT(false, "Can't change scene during playback");
    }

#endif
}

void LiveAudioEngine::Private::updateScene()
{
    MO_DEBUG("LiveAudioEngine::Private::updateScene() "
             "engine->scene() == " << engine->scene()
             << " playback == " << parent->isPlayback());

    if (engine->scene() && engine->config() != defaultConf)
        engine->setScene(engine->scene(), defaultConf, engine->thread());
}

bool LiveAudioEngine::isAudioConfigured() const
{
    return AUDIO::AudioDevice::isAudioConfigured();
}

bool LiveAudioEngine::isAudioInitialized() const
{
    return p_->audioDevice && p_->audioDevice->isOk();
}

bool LiveAudioEngine::isPlayback() const
{
    return p_->audioDevice && p_->audioDevice->isPlaying();
}

bool LiveAudioEngine::isPause() const { return p_->isPause; }

bool LiveAudioEngine::initAudioDevice()
{
    MO_DEBUG_AUDIO("LiveAudioEngine::initAudioDevice()");

    if (isAudioInitialized())
    {
        MO_WARNING("LiveAudioEngine::initAudioDevice() called twice!");
        return true;
    }

    if (!p_->audioDevice)
        p_->audioDevice = new AUDIO::AudioDevice();

    if (!p_->audioDevice->isAudioConfigured())
    {
        QMessageBox::information(0, tr("Audio device"),
                                 tr("No audio device has been configured yet."));
        return false;
    }

    if (!p_->audioDevice->initFromSettings())
        return false;

    // get audio configuration
    p_->defaultConf = p_->audioDevice->configuration();
    p_->updateScene();

    // install callback
    using namespace std::placeholders;
    p_->audioDevice->setCallback(std::bind(
        &LiveAudioEngine::Private::audioCallback, p_, _1, _2, _3));

    //isFirstAudioCallback_ = true;
    return true;
}

void LiveAudioEngine::closeAudioDevice()
{
    if (isPlayback())
        stop();

    if (isAudioInitialized())
        p_->audioDevice->close();
}

bool LiveAudioEngine::start()
{
    p_->isPause = false;

    if (isPlayback())
        return true;

    // init device
    if (!isAudioInitialized())
        if (!initAudioDevice())
            return false;

    // init communication stuff
    p_->audioOutQueue.reset();

#ifdef MO_BUFFER_TRICK
    Private::BufferTime bt;
    bt.sample = pos();
    bt.time = systemTime();
    p_->lastBuffer = bt;
#endif

    // init threads
    /*
    p_->audioInThread = new AudioEngineInThread(this, this);
    p_->audioInThread->start();
    */
    if (!p_->audioOutThread)
        p_->audioOutThread = new AudioEngineOutThread(this, this);

    // start threads
    p_->audioOutThread->start();

    // start device
    try
    {
        p_->timeOffset = p_->engine->second();
        p_->audioDevice->start();
        p_->startTime = applicationTime();
    }
    catch (const Exception& e)
    {
        p_->audioOutThread->stop();

        QMessageBox::critical(0, tr("Audio device"),
                              tr("Could initialize but not start audio playback.\n%1")
                              .arg(e.what()));
        return false;
    }

    return true;
}

void LiveAudioEngine::stop()
{
    // kill audio-out thread
    if (p_->audioOutThread)
    {
        if (p_->audioOutThread->isRunning())
            p_->audioOutThread->stop();
        p_->audioOutThread->deleteLater();
        p_->audioOutThread = 0;
    }

    p_->audioDevice->stop();
}

void LiveAudioEngine::pause(bool enable)
{
    p_->isPause = enable;
}


void LiveAudioEngine::Private::audioCallback(
        const F32 * in, F32 * out, const AUDIO::AudioDevice::StreamTime &
#ifdef MO_BUFFER_TRICK
        st
#endif
        )
{
#ifdef MO_BUFFER_TRICK
    lastBuffer = BufferTime(curSample, applicationTime() + st.outputTime);
    curSample += engine->config().bufferSize();
    //MO_PRINT("audio callback " << st.inputTime << " " << st.currentTime << " " << st.outputTime);
#endif

    // get input
    audioInQueue.produce(in);

    // ---- process output ----

    // get output from AudioOutThread
    const F32 * buf;
    if (audioOutQueue.consume(buf))
    {
        memcpy(out, buf, engine->config().bufferSize()
                            * engine->config().numChannelsOut()
                            * sizeof(F32));
    }
    else
    {
        memset(out, 0, engine->config().bufferSize()
                            * engine->config().numChannelsOut()
                            * sizeof(F32));
        //MO_WARNING("audio-out buffer underrun");
    }
}


} // namespace MO
