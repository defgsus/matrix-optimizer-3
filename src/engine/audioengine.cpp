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
          curSample     (0)
    { }

    void setup();

    AudioEngine * engine;
    Scene * scene;

    AUDIO::Configuration conf;
    ObjectDspPath path;
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

void AudioEngine::setScene(Scene * s, const AUDIO::Configuration & conf)
{
    p_->conf = conf;
    p_->scene = s;
    p_->setup();
}

void AudioEngine::Private::setup()
{
    path.createPath(scene, conf);
}

void AudioEngine::process(const F32 *inputs, F32 *outputs)
{

}



} // namespace MO
