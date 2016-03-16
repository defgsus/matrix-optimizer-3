/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/16/2016</p>
*/

#include "synthao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/synth.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/util/synthsetting.h"
#include "io/datastream.h"
#include "io/error.h"


namespace MO {

MO_REGISTER_OBJECT(SynthAO)

class SynthAO::Private
{
    public:

    Private(SynthAO * ao)
        : ao        (ao)
        , synthSet  (ao)
    { }

    SynthAO * ao;

    SynthSetting synthSet;
};


SynthAO::SynthAO()
    : AudioObject   (),
      p_            (new Private(this))
{
    setName("Synth");
    setNumberAudioInputsOutputs(0, 1);
}

SynthAO::~SynthAO()
{
    delete p_;
}

void SynthAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aosynth", 1);
}

void SynthAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aosynth", 1);
}

void SynthAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("synth", tr("synthesizer"));
    initParameterGroupExpanded("synth");

        p_->synthSet.createParameters("");

    params()->endParameterGroup();
}

void SynthAO::onParametersLoaded()
{
    p_->synthSet.onParametersLoaded();
}

void SynthAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    p_->synthSet.updateParameterVisibility();
}

void SynthAO::onParameterChanged(Parameter *p)
{
    AudioObject::onParameterChanged(p);

    p_->synthSet.onParameterChanged(p);
}

void SynthAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);

}


void SynthAO::processAudio(const RenderTime& time)
{
    const QList<AUDIO::AudioBuffer*>&
            outputs = audioOutputs(time.thread());

    p_->synthSet.feedSynth(time);
    if (outputs[0])
        p_->synthSet.synth()->process(
                    outputs[0]->writePointer(), outputs[0]->blockSize());
}

} // namespace MO
