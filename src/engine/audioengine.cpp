/** @file audioengine.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#include "audioengine.h"
#include "object/scene.h"
//#include "object/scenelock_p.h"
#include "object/util/objectdsppath.h"
#include "object/util/audioobjectconnections.h"
#include "audio/configuration.h"
#include "audio/tool/audiobuffer.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

// ################################## AudioEngine::Private ########################################

class AudioEngine::Private
{
public:

    Private(AudioEngine * e)
        : engine        (e),
          scene         (0),
          threadIdx     (0),
          curSample     (0)
    { }

    void setup();

    AudioEngine * engine;
    Scene * scene;
    AUDIO::Configuration conf;
    ObjectDspPath path;
    uint threadIdx;
    SamplePos curSample;
};





// ################################## AudioEngine implementation ########################################


AudioEngine::AudioEngine(QObject *parent)
    : QObject       (parent),
      p_            (new Private(this))
{
}

AudioEngine::~AudioEngine()
{
    delete p_;
}


Scene * AudioEngine::scene() const
{
    return p_->scene;
}

uint AudioEngine::thread() const
{
    return p_->threadIdx;
}

const AUDIO::Configuration& AudioEngine::config() const
{
    return p_->conf;
}

SamplePos AudioEngine::pos() const
{
    return p_->curSample;
}

Double AudioEngine::second() const
{
    return p_->conf.sampleRateInv() * p_->curSample;
}


void AudioEngine::seek(SamplePos pos)
{
    p_->curSample = pos;
}

void AudioEngine::setScene(Scene * s, const AUDIO::Configuration & conf, uint thread)
{
    p_->conf = conf;
    p_->scene = s;
    p_->threadIdx = thread;
    p_->setup();
}

void AudioEngine::Private::setup()
{
    MO_DEBUG("AudioEngine::setup()");

    path.createPath(scene, conf);

#ifdef MO_ENABLE_DEBUG
    path.dump(std::cout);
#endif
}

void AudioEngine::process(const F32 *, F32 * outputs)
{
    // apply all transformations
    p_->path.calcTransformations(p_->curSample, p_->threadIdx);

    // run audio block
    p_->path.calcAudio(p_->curSample, p_->threadIdx);

    // advance scene time
    p_->curSample += p_->conf.bufferSize();

    // copy output buffers
    for (const AUDIO::AudioBuffer * b : p_->path.audioOutputs())
    {
        b->readBlock(outputs);
        outputs += p_->conf.bufferSize();
    }
}

void AudioEngine::processForDevice(const F32 *, F32 * outputs)
{
    // apply all transformations
    p_->path.calcTransformations(p_->curSample, p_->threadIdx);

    // run audio block
    p_->path.calcAudio(p_->curSample, p_->threadIdx);

    // advance scene time
    p_->curSample += p_->conf.bufferSize();

    // copy output buffers
    for (const AUDIO::AudioBuffer * b : p_->path.audioOutputs())
    {
        // write channel-interlaced
        b->readBlock(outputs, p_->conf.numChannelsOut());
        // advance channel
        ++outputs;
    }
}


} // namespace MO
