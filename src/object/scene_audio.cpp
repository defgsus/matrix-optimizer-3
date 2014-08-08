/** @file scene_audio.cpp

    @brief Audio-part of Scene implementation

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "scene.h"
#include "scenelock_p.h"
#include "io/error.h"
#include "io/log.h"
#include "audio/audiodevice.h"
#include "audio/audiosource.h"
#include "object/audio/audiounit.h"
#include "object/microphone.h"

namespace MO {

// ---------------------- device handling --------------------------


bool Scene::isAudioInitialized() const
{
    return audioDevice_->ok();
}

void Scene::initAudioDevice_()
{
    if (audioDevice_->isAudioConfigured())
    {
        if (!audioDevice_->initFromSettings())
            return;

        sceneSampleRate_ = audioDevice_->sampleRate();
        sceneBufferSize_[MO_AUDIO_THREAD] = audioDevice_->bufferSize();

        updateBufferSize_();
        updateSampleRate_();
        updateAudioBuffers_();

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

void Scene::audioCallback_(const F32 *, F32 * out)
{
    if (isFirstAudioCallback_)
    {
        setCurrentThreadName("AUDIO");
        isFirstAudioCallback_ = false;
    }
    MO_ASSERT(audioDevice_->bufferSize() == bufferSize(MO_AUDIO_THREAD),
              "buffer-size mismatch");

    calculateAudioBlock(samplePos_, MO_AUDIO_THREAD);
    getAudioOutput(audioDevice_->numOutputChannels(), MO_AUDIO_THREAD, out);

    // update scene time
    setSceneTime(samplePos_ + bufferSize(MO_AUDIO_THREAD));
}


// ------------------- initialization --------------------------

void Scene::setBufferSize(uint bufferSize, uint thread)
{
    Object::setBufferSize(bufferSize, thread);
}



// ------------------ audio processing -------------------------


void Scene::calculateAudioBlock(SamplePos samplePos, uint thread)
{
    ScopedSceneLockRead lock(this);

    const uint size = bufferSize(thread);

    const Double time = sampleRateInv() * samplePos;
    Double rtime = time;

    // calculate one block of transformations
    // XXX needs only be done for audioobjects and microphones
    for (uint i = 0; i<size; ++i)
    {
        calculateSceneTransform_(thread, i, rtime);
        rtime += sampleRateInv();
    }

    // calculate audio objects
    for (auto o : audioObjects_)
    {
        o->performAudioBlock(samplePos, thread);
        // fill delay lines
        for (auto a : o->audioSources())
            a->pushDelay(thread);
    }

    for (int i = 0; i<microphones_.size(); ++i)
    {
        auto mic = microphones_[i];

        // clear audio buffer
        F32 * buffer = &audioOutput_[thread][i * size];
        memset(buffer, 0, sizeof(F32) * size);

        // for each object
        for (auto o : audioObjects_)
            // for each source in object
            for (auto src : o->audioSources())
                // add to microphone 'membrane'
                mic->sampleAudioSource(src, buffer, thread);
    }
}

void Scene::getAudioOutput(uint numChannels, uint thread, F32 *buffer) const
{
    const uint size = bufferSize(thread);

    // clear buffer
    memset(buffer, 0, sizeof(F32) * size * numChannels);

    // rearange the audioOutput buffer

    const F32* src = &audioOutput_[thread][0];

    const uint chan = std::min(numChannels, (uint)microphones_.size());
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


} // namespace MO
