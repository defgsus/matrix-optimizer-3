/** @file liveaudioengine.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#include <QThread>

#include "liveaudioengine.h"
#include "object/scene.h"
#include "object/scenelock_p.h"
#include "object/util/audioobjectconnections.h"
#include "tool/locklessqueue.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {








namespace {

    // ################################ audio in worker thread #########################################

    class AudioInThread : public QThread
    {
    public:
        AudioInThread(LiveAudioEngine * engine, QObject * parent)
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

    class AudioOutThread : public QThread
    {
    public:
        AudioOutThread(LiveAudioEngine * engine, QObject * parent)
            : QThread   (parent),
              engine_   (engine),
              stop_     (false)
        {

        }

        void stop() { stop_ = true; wait(); }

        void run()
        {
            setCurrentThreadName("AUDIO_OUT");
            MO_DEBUG_AUDIO("AudioOutThread::run()");
#if 0
            const uint
                    bufferLength = scene_->bufferSize(MO_AUDIO_THREAD),
                    numChannelsOut = scene_->numberChannelsOut(),
                    bufferSize = bufferLength * numChannelsOut,
                    numAhead = 4;
            uint writepos = 0;

            std::vector<F32> buffer(bufferSize * numAhead);
            scene_->audioOutQueue_->reset();

            // length of a buffer in microseconds
            const unsigned long bufferTimeU =
                1000000 * scene_->bufferSize(MO_AUDIO_THREAD) / scene_->sampleRate();

            while (!stop_)
            {
                //std::cerr << scene_->audioQueue_->count() << std::endl;

                if (scene_->audioOutQueue_->count() < numAhead)
                {

                    // calculate an audio block (locked)
                    scene_->calculateAudioBlock(scene_->samplePos_, MO_AUDIO_THREAD);

                    // update output envelopes
                    scene_->updateOutputEnvelopes_(MO_AUDIO_THREAD);

                    // write buffer here
                    F32 * buf = &buffer[writepos];
                    // advance next buffer-write
                    writepos += bufferSize;
                    if (writepos >= buffer.size())
                        writepos = 0;

                    // rearrange the output buffer for the device
                    scene_->getAudioOutput(numChannelsOut, MO_AUDIO_THREAD, buf);

                    // publish
                    scene_->audioOutQueue_->produce(buf);

                    // advance scene time
                    scene_->setSceneTime(scene_->samplePos_ + bufferLength);
                }
                else
                    usleep(bufferTimeU);
            }
#endif
            MO_DEBUG_AUDIO("AudioOutThread::run() finished");
        }

    private:

        LiveAudioEngine * engine_;

        volatile bool stop_;
    };


} // namespace










// ################################## LiveAudioEngine::Private ########################################

class LiveAudioEngine::Private
{
public:

    Private(LiveAudioEngine * e)
        : engine        (e),
          scene         (0)
    { }

    void setScene(Scene*);

    LiveAudioEngine * engine;
    Scene * scene;

    AUDIO::AudioDevice * audioDevice_;

    //AudioInThread * audioInThread;
    //AudioOutThread * audioOutThread;
    LocklessQueue<const F32*> * audioInQueue;
    LocklessQueue<F32*> * audioOutQueue;
};





// ################################## LiveAudioEngine implementation ########################################


LiveAudioEngine::LiveAudioEngine(QObject *parent)
    : QObject       (parent),
      p_            (new Private(this))
{
}

LiveAudioEngine::~LiveAudioEngine()
{
    delete p_;
}



void LiveAudioEngine::setScene(Scene * s)
{
    p_->setScene(s);
}

void LiveAudioEngine::Private::setScene(Scene * s)
{
    scene = s;
}


} // namespace MO
