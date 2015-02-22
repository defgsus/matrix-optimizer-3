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
          curSample     (0),
          isPathPrepared(false)
    { }

    void setup();

    AudioEngine * engine;
    Scene * scene;
    AUDIO::Configuration conf;
    ObjectDspPath path;
    uint threadIdx;
    SamplePos curSample;

    bool isPathPrepared;
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
    p_->isPathPrepared = false;
    p_->conf = conf;
    p_->scene = s;
    p_->threadIdx = thread;
    p_->setup();
}

void AudioEngine::Private::setup()
{
    MO_DEBUG("AudioEngine::setup()");

    path.createPath(scene, conf, threadIdx);

#ifdef MO_ENABLE_DEBUG
    path.dump(std::cout);
#endif
}

void AudioEngine::prepareUdp()
{
    // update network objects
    if (!p_->isPathPrepared)
    {
        p_->path.preparePath();
        p_->isPathPrepared = true;
    }
}

void AudioEngine::process(const F32 * inputs, F32 * outputs)
{
    // update network objects
    if (!p_->isPathPrepared)
    {
        p_->path.preparePath();
        p_->isPathPrepared = true;
    }

    // copy into input buffers
    for (AUDIO::AudioBuffer * b : p_->path.audioInputs())
    {
        b->writeBlock(inputs);
        inputs += config().bufferSize();
    }

    // apply all transformations
    p_->path.calcTransformations(p_->curSample);

    // run audio block
    p_->path.calcAudio(p_->curSample);

    // advance scene time
    p_->curSample += p_->conf.bufferSize();

    // copy output buffers
    for (const AUDIO::AudioBuffer * b : p_->path.audioOutputs())
    {
        b->readBlock(outputs);
        outputs += config().bufferSize();
    }
}

void AudioEngine::processForDevice(const F32 * inputs, F32 * outputs)
{
    // update network objects
    if (!p_->isPathPrepared)
    {
        p_->path.preparePath();
        p_->isPathPrepared = true;
    }

    // copy into input buffers
    if (inputs)
    for (AUDIO::AudioBuffer * b : p_->path.audioInputs())
    {
        // write channel-deinterlaced
        b->writeBlock(inputs, config().numChannelsIn());
        // advance channel
        ++inputs;
    }

    // apply all transformations
    p_->path.calcTransformations(p_->curSample);

    // run audio block
    p_->path.calcAudio(p_->curSample);

    // advance scene time
    p_->curSample += p_->conf.bufferSize();

    // copy output buffers
    for (const AUDIO::AudioBuffer * b : p_->path.audioOutputs())
    {
        // write channel-interlaced
        b->readBlock(outputs, config().numChannelsOut());
        // advance channel
        ++outputs;
    }
}


} // namespace MO
