/** @file liveaudioengine.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#include <QThread>
#include <QMessageBox>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "liveaudioengine.h"
#include "object/scene.h"
#include "object/scenelock_p.h"
#include "object/util/audioobjectconnections.h"
#include "tool/locklessqueue.h"
#include "audio/audiodevice.h"
#include "audio/configuration.h"
#include "audio/tool/audiobuffer.h"
#include "engine/audioengine.h"
#include "io/error.h"
#include "io/log.h"

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
          engine        (new AudioEngine()),
          nextEngine    (0),
          disposeEngine (0),
          audioDevice   (0),
          defaultConf   (44100, 256, 0, 2),
          audioOutThread(0)
    { }

    ~Private()
    {
        delete audioDevice;
        delete disposeEngine;
        delete nextEngine;
        delete engine;
    }

    /** Recreates the AudioEngine dsp path */
    void updateScene();
    void audioCallback(const F32 *, F32 *);

    LiveAudioEngine * parent;
    AudioEngine * engine, * nextEngine, * disposeEngine;

    AUDIO::AudioDevice * audioDevice;
    AUDIO::Configuration defaultConf;

    QReadWriteLock engineLock;

    //AudioInThread * audioInThread;
    AudioEngineOutThread * audioOutThread;
    LocklessQueue<const F32*> audioInQueue;
    LocklessQueue<const F32*> audioOutQueue;
};








// ################################ audio in worker thread #########################################

class AudioEngineInThread : public QThread
{
public:
    AudioEngineInThread(LiveAudioEngine * engine, QObject * parent)
        : QThread   (parent),
          engine_   (engine),
          stop_     (false)
    {

    }

    /** Blocking stop */
    void stop() { stop_ = true; wait(); }

    void run()
    {
        setCurrentThreadName("AUDIO_IN");
        MO_DEBUG_AUDIO("AudioInThread::run()");
#if (0)
        /*
        const uint
                bufferLength = scene_->bufferSize(MO_AUDIO_THREAD),
                numChannelsIn = scene_->numberChannelsIn(),
                bufferSize = bufferLength * numChannelsIn,
                numAhead = scene_->numInputBuffers_;*/

        scene_->audioInQueue_->reset();

        const F32* buf;

        // length of a buffer in microseconds
        const unsigned long bufferTimeU =
            1000000 * scene_->bufferSize(MO_AUDIO_THREAD) / scene_->sampleRate();

        while (!stop_)
        {
            // XXX sometimes crash on load-new-scene
            if (scene_->audioInQueue_->consume(buf))
            {
                // process audio input
                if (!scene_->topLevelAudioUnits_.empty())
                {
                    // transform api [bufferSize][channels] to [channels][bufferSize]
                    scene_->transformAudioInput_(buf, MO_AUDIO_THREAD);

                    {
                        ScopedSceneLockRead lock(scene_);
                        // process with AudioUnits
                        scene_->processAudioInput_(MO_AUDIO_THREAD);
                    }
                }
            }
            else usleep(bufferTimeU);
        }
#endif
        MO_DEBUG_AUDIO("AudioInThread::run() finished");
    }

private:

    LiveAudioEngine * engine_;

    volatile bool stop_;
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
            // check for engine swap
            {
                QReadLocker lock(&engine_->p_->engineLock);

                // while in lock, swap engine
                if (live->nextEngine)
                {
                    live->disposeEngine = live->engine;
                    live->engine = live->nextEngine;
                    live->nextEngine = 0;

                    // update local settings
                    bufferSize = engine_->config().bufferSize(),
                    numChannelsOut = engine_->config().numChannelsOut(),
                    bufferSizeChan = bufferSize * numChannelsOut,
                    bufferTimeU = 1000000 *
                            bufferSize * engine_->config().sampleRateInv();

                    bufferForDevice.setSize(bufferSizeChan, numAhead);
                }
            }

            // calc buffers for next system-out callback
            if (live->audioOutQueue.count() < numAhead)
            {
                {
                    ScopedSceneLockRead lock(engine_->scene());

                    // calculate an audio block
                    live->engine->processForDevice(
                                0,
                                bufferForDevice.writePointer());
                }

                // publish
                bufferForDevice.nextBlock();
                live->audioOutQueue.produce(
                            bufferForDevice.readPointer());

            }
            else
                usleep(bufferTimeU);
        }

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
    return p_->engine->second();
}


void LiveAudioEngine::seek(SamplePos pos)
{
    p_->engine->seek(pos);
}


void LiveAudioEngine::setScene(Scene * s, uint thread)
{
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
}

void LiveAudioEngine::Private::updateScene()
{
    if (engine->scene())
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
        &LiveAudioEngine::Private::audioCallback, p_, _1, _2));

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
    if (isPlayback())
        return true;

    // init device
    if (!isAudioInitialized())
        if (!initAudioDevice())
            return false;

    // init communication stuff
    p_->audioOutQueue.reset();

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
        p_->audioDevice->start();
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(0, tr("Audio device"),
                              tr("Could initialized but not start audio playback.\n%1")
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

void LiveAudioEngine::Private::audioCallback(const F32 * , F32 * out)
{
    // ---- process output ----

    // get output from AudioOutThread
    const F32 * buf;
    if (audioOutQueue.consume(buf))
        memcpy(out, buf, engine->config().bufferSize()
                            * engine->config().numChannelsOut()
                            * sizeof(F32));
    else
    {
        memset(out, 0, engine->config().bufferSize()
                            * engine->config().numChannelsOut()
                            * sizeof(F32));
        //MO_WARNING("audio-out buffer underrun");
    }
}


} // namespace MO
