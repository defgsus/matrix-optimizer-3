/** @file synthesizer.cpp

    @brief Basic, non-graphic synthesizer object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#include "synthesizer.h"
#include "io/datastream.h"
#include "audio/audiosource.h"
#include "param/parameterfloat.h"
#include "param/parameterint.h"
#include "param/parameterselect.h"
#include "io/log.h"
#include "audio/tool/synth.h"
#include "util/synthsetting.h"

namespace MO {

MO_REGISTER_OBJECT(Synthesizer)

Synthesizer::Synthesizer(QObject *parent)
    : Object    (parent),
      synth_    (new SynthSetting(this))
{
    setName("Synthesizer");
}

void Synthesizer::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("syn", 1);
}

void Synthesizer::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("syn", 1);
}

void Synthesizer::createParameters()
{
    Object::createParameters();

    beginParameterGroup("synthesizer", tr("synthesizer"));

        synth_->createParameters("");

    endParameterGroup();
}

void Synthesizer::updateParameterVisibility()
{
    Object::updateParameterVisibility();

    synth_->updateParameterVisibility();
}

void Synthesizer::onParameterChanged(Parameter * p)
{
    Object::onParameterChanged(p);

    synth_->onParameterChanged(p);
}

void Synthesizer::onParametersLoaded()
{
    Object::onParametersLoaded();

    synth_->onParametersLoaded();
}

void Synthesizer::createAudioSources()
{
    Object::createAudioSources();

    audio_ = createAudioSource();
}

void Synthesizer::setSampleRate(uint samplerate)
{
    Object::setSampleRate(samplerate);

    synth_->synth()->setSampleRate(samplerate);
}

void Synthesizer::updateAudioTransformations(Double, uint thread)
{
    audio_->setTransformation(transformation(thread, 0), thread, 0);
}

void Synthesizer::updateAudioTransformations(Double, uint , uint thread)
{
    // copy the block of transformations
    audio_->setTransformation(transformations(thread), thread);
}

void Synthesizer::performAudioBlock(SamplePos pos, uint thread)
{
    const Double time = pos * sampleRateInv();

    synth_->feedSynth(time, thread);

    synth_->synth()->process(audio_->samples(thread), bufferSize(thread));
}


} // namespace MO

