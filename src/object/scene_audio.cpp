/** @file scene_audio.cpp

    @brief Audio-part of Scene implementation

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include <QThread>
#include <QTime>

#include "scene.h"
#include "scenelock_p.h"
#include "io/error.h"
#include "io/log.h"
#include "audio/audiodevice.h"
#include "audio/audiosource.h"
#include "audio/audiomicrophone.h"
#include "audio/tool/envelopefollower.h"
#include "object/audio/audiounit.h"
#include "object/microphone.h"
#include "tool/locklessqueue.h"

namespace MO {

// ------------------- audio in worker thread -------------------------

class AudioInThread : public QThread
{
public:
    AudioInThread(Scene * scene, QObject * parent)
        : QThread   (parent),
          scene_    (scene),
          stop_     (false)
    {

    }

    void stop() { stop_ = true; wait(); }

    void run()
    {
        setCurrentThreadName("AUDIO_IN");
        MO_DEBUG_AUDIO("AudioInThread::run()");

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

        MO_DEBUG_AUDIO("AudioInThread::run() finished");
    }

private:

    Scene * scene_;

    volatile bool stop_;
};





// ------------------- audio out worker thread -------------------------

class AudioOutThread : public QThread
{
public:
    AudioOutThread(Scene * scene, QObject * parent)
        : QThread   (parent),
          scene_    (scene),
          stop_     (false)
    {

    }

    void stop() { stop_ = true; wait(); }

    void run()
    {
        setCurrentThreadName("AUDIO_OUT");
        MO_DEBUG_AUDIO("AudioOutThread::run()");

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

        MO_DEBUG_AUDIO("AudioOutThread::run() finished");
    }

private:

    Scene * scene_;

    volatile bool stop_;
};




// ---------------------- device handling --------------------------


bool Scene::isAudioInitialized() const
{
    return audioDevice_->ok();
}

void Scene::initAudioDevice_()
{
    MO_DEBUG_AUDIO("Scene::initAudioDevice_()");

    if (audioDevice_->isAudioConfigured())
    {
        if (!audioDevice_->initFromSettings())
            return;

        sceneSampleRate_ = audioDevice_->sampleRate();
        sceneBufferSize_[MO_AUDIO_THREAD] = audioDevice_->bufferSize();
        numInputChannels_ = audioDevice_->numInputChannels();
        numOutputChannels_ = audioDevice_->numOutputChannels();

        updateBufferSize_();
        updateSampleRate_();
        updateAudioBuffers_();
        prepareAudioInputBuffer_(MO_AUDIO_THREAD);
        allocateAudioOutputEnvelopes_(MO_AUDIO_THREAD);

        updateAudioUnitChannels_();

        emit numberChannelsChanged(numInputChannels_, numOutputChannels_);

        using namespace std::placeholders;
        audioDevice_->setCallback(std::bind(
            &Scene::audioCallback_, this, _1, _2));

        isFirstAudioCallback_ = true;
    }
}

void Scene::closeAudio()
{
    if (isPlayback())
        stop();

    if (isAudioInitialized())
        audioDevice_->close();
}

void Scene::audioCallback_(const F32 * in, F32 * out)
{
    if (isFirstAudioCallback_)
    {
        setCurrentThreadName("AUDIO_API");
        isFirstAudioCallback_ = false;
    }

    //MO_ASSERT(audioDevice_->bufferSize() == bufferSize(MO_AUDIO_THREAD),
    //          "buffer-size mismatch");

    // ---- process input ----

    if (audioInQueue_->count() < numInputBuffers_)
    {
        // put into input-buffer
        const uint inputBufferSize = numberChannelsIn() * bufferSize(MO_AUDIO_THREAD);
        memcpy(&apiAudioInputBuffer_[curInputBuffer_ * inputBufferSize],
                in, inputBufferSize * sizeof(F32));
        // get next write-pos
        curInputBuffer_ = (curInputBuffer_ + 1) % numInputBuffers_;
        // publish to AudioInThread
        audioInQueue_->produce(in);
    }

    // ---- process output ----

    // get output from AudioOutThread
    F32 * buf;
    if (audioOutQueue_->consume(buf))
        memcpy(out, buf, bufferSize(MO_AUDIO_THREAD) * numberChannelsOut() * sizeof(F32));
    else
    {
        memset(out, 0, bufferSize(MO_AUDIO_THREAD) * numberChannelsOut() * sizeof(F32));
        //MO_WARNING("audio-out buffer underrun");
    }

    /*
    // process audio input
    if (!topLevelAudioUnits_.empty())
    {
        transformAudioInput_(in, MO_AUDIO_THREAD);
        processAudioInput_(MO_AUDIO_THREAD);
    }

    // perform audio-process of whole scene tree
    calculateAudioBlock(samplePos_, MO_AUDIO_THREAD);

    // update output envelopes
    updateOutputEnvelopes_(MO_AUDIO_THREAD);

    // rearrange the output buffer for the device
    getAudioOutput(audioDevice_->numOutputChannels(), MO_AUDIO_THREAD, out);

    // update scene time
    setSceneTime(samplePos_ + bufferSize(MO_AUDIO_THREAD));
    */
}



void Scene::start()
{
    ScopedSceneLockWrite lock(this);

    if (!isAudioInitialized())
        initAudioDevice_();

    updateAudioUnitChannels_();

    if (isAudioInitialized())
    {
        isPlayback_ = true;

        audioInThread_ = new AudioInThread(this, this);
        audioInThread_->start();
        audioOutThread_ = new AudioOutThread(this, this);
        audioOutThread_->start();

        audioDevice_->start();

        emit playbackStarted();
    }
}

void Scene::stop()
{
    if (isPlayback())
    {
        isPlayback_ = false;
        if (isAudioInitialized())
            audioDevice_->stop();

        emit playbackStopped();
    }
    else
    {
        setSceneTime(0.0);
    }

    // kill audio-in thread
    if (audioInThread_)
    {
        if (audioInThread_->isRunning())
            audioInThread_->stop();
        audioInThread_->deleteLater();
        audioInThread_ = 0;
    }

    // kill audio-out thread
    if (audioOutThread_)
    {
        if (audioOutThread_->isRunning())
            audioOutThread_->stop();
        audioOutThread_->deleteLater();
        audioOutThread_ = 0;
    }

    /*
    if (timer_.isActive())
        timer_.stop();
    else
        setSceneTime(0);
    */
}


// ------------------- initialization --------------------------

void Scene::setBufferSize(uint bufferSize, uint thread)
{
    Object::setBufferSize(bufferSize, thread);
}

void Scene::prepareAudioInputBuffer_(uint thread)
{
    sceneAudioInput_.resize(numInputChannels_ * bufferSize(thread));
    apiAudioInputBuffer_.resize(
                numInputBuffers_ * bufferSize(thread) * numInputChannels_);
}

void Scene::updateAudioUnitChannels()
{
    ScopedSceneLockWrite lock(this);

    updateAudioUnitChannels_();
}

void Scene::updateAudioUnitChannels_()
{
    MO_DEBUG_AUDIO("Scene::updateAudioUnitChannels_() "
                   "top-level units == " << topLevelAudioUnits_.size());

    for (AudioUnit * au : topLevelAudioUnits_)
    {
        if (au->numChannelsIn() != numInputChannels_
            || au->needsChannelChange())
            au->setNumChannelsIn(numInputChannels_);

        au->updateSubUnitChannels();
    }
}

void Scene::updateDelaySize_()
{
    for (auto o : audioObjects_)
        for (auto a : o->audioSources())
            for (uint i=0; i<numberThreads(); ++i)
                a->setDelaySize(sceneDelaySize_[i], i);
}

void Scene::updateAudioBuffers_()
{
    MO_DEBUG_AUDIO("Scene::updateAudioBuffers_() numberThreads() == " << numberThreads());

    sceneAudioOutput_.resize(numberThreads());

    for (uint i=0; i<numberThreads(); ++i)
    {
        sceneAudioOutput_[i].resize(bufferSize(i) * numMicrophones());

        memset(&sceneAudioOutput_[i][0], 0, sizeof(F32) * bufferSize(i) * numMicrophones());

        MO_DEBUG_AUDIO("audioOutput_[" << i << "].size() == "
                       << sceneAudioOutput_[i].size());
    }
}

void Scene::allocateAudioOutputEnvelopes_(uint /*thread*/)
{
    const uint num = numMicrophones();

    outputEnvelopes_.resize(num);

    for (auto e : outputEnvelopeFollower_)
        delete e;

    outputEnvelopeFollower_.clear();

    for (uint i=0; i<num; ++i)
    {
        outputEnvelopeFollower_.push_back(
                    new AUDIO::EnvelopeFollower() );
    }

    emit numberOutputEnvelopesChanged(num);
}

// ------------------ audio out processing -------------------------


void Scene::calculateAudioBlock(SamplePos samplePos, uint thread)
{
    ScopedSceneLockRead lock(this);

    const uint size = bufferSize(thread);

    const Double time = sampleRateInv() * samplePos;
    Double rtime = time;

    // calculate one block of transformations
#if (1)
    for (uint i = 0; i<size; ++i)
    {
        calculateAudioSceneTransform_(thread, i, rtime);
        rtime += sampleRateInv();
    }
#elif (0) // NOT WORKING RIGHT
    // calculate only a few transformations
    const uint reduce = 20;
    const uint rsize = size/reduce;
    for (uint i = 0; i<rsize; ++i)
    {
        calculateAudioSceneTransform_(thread, i*reduce, rtime);
        rtime += sampleRateInv() * reduce;
    }
    // calculate the rest if buffersize/reduce misses something
    if (rsize < size-1)
    for (uint i=rsize; i<size; ++i)
    {
        calculateAudioSceneTransform_(thread, i, rtime);
        rtime += sampleRateInv();
    }
    // interpolate in between
    for (Object * o : posObjectsAudio_)
    {
        for (uint i=0; i<rsize; ++i)
        {
            const Mat4&
                    trans0 = o->transformation(thread, i*reduce),
                    trans1 = o->transformation(thread, (i+1)*reduce);
            for (uint j=1; j<reduce; ++j)
            {
                const Float t = Float(j) / (reduce);
                o->setTransformation(thread, i*reduce+j,
                        trans0 + t * (trans1 - trans0));
            }
        }
    }
#else
    calculateAudioSceneTransform_(thread, 0, sampleRateInv() * samplePos);
    calculateAudioSceneTransform_(thread, size-1, sampleRateInv() * (samplePos + size-1));
#endif

    // calculate audio objects
    for (auto o : audioObjects_)
    {
        o->performAudioBlock(samplePos, thread);
        o->updateAudioTransformations(time, size, thread);

        // fill delay lines
        for (auto a : o->audioSources())
            a->pushDelay(thread);
    }

    int mici = 0;
    for (auto o : microphoneObjects_)
    {
        o->updateAudioTransformations(time, size, thread);

        for (auto mic : o->microphones())
        {
            mic->clearOutputBuffer(thread);

            // for each audio object
            for (auto o : audioObjects_)
                // for each source in object
                for (auto src : o->audioSources())
                    // add to microphone 'membrane'
                    mic->sampleAudioSource(src, thread);

            // copy back to scene output buffer
            memcpy(&sceneAudioOutput_[thread][size*mici],
                    mic->outputBuffer(thread),
                    size * sizeof(F32));

            mici++;
        }
    }


}

void Scene::getAudioOutput(uint numChannels, uint thread, F32 *buffer) const
{
    const uint size = bufferSize(thread);

    // clear buffer
    memset(buffer, 0, sizeof(F32) * size * numChannels);

    // rearrange the audioOutput buffer

    const F32* src = &sceneAudioOutput_[thread][0];

    const uint chan = std::min(numChannels, numMicrophones());
    for (uint b = 0; b < size; ++b)
    {
        for (uint c = 0; c < chan; ++c)
        {
            *buffer++ = src[c * size + b];
        }

        // skip unused channels
        buffer += (numChannels - chan);
    }
}


void Scene::updateOutputEnvelopes_(uint thread)
{
    const uint bsize = bufferSize(thread),
               nummic = numMicrophones();

    for (uint i = 0; i < nummic; ++i)
    {
        outputEnvelopes_[i] =
            outputEnvelopeFollower_[i]->process(
                    &sceneAudioOutput_[thread][i * bsize],
                    bsize);
    }

    emit outputEnvelopeChanged(&outputEnvelopes_[0]);
}

// ------------------- audio in processing --------------------------

void Scene::transformAudioInput_(const F32 *in, uint thread)
{
    const uint bsize = bufferSize(thread);

    for (uint i=0; i<bsize; ++i)
        for (uint c=0; c<numInputChannels_; ++c)
            sceneAudioInput_[c * bsize + i] = *in++;
}

void Scene::processAudioInput_(uint thread)
{
    //const uint bsize = bufferSize(thread);

    for (AudioUnit * au : topLevelAudioUnits_)
        au->processAudioBlock_(&sceneAudioInput_[0], sceneTime_, thread, true);
}

} // namespace MO
