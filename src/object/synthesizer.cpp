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

    beginParameterGroup("audiosources", tr("audio sources"));

        p_polyAudio_ = createBooleanParameter("polyaudiosrc", tr("polyphonic audio sources"),
                                              tr("When enabled, each synthesizer voice will be an audio source"),
                                              tr("All voices are emitted through one audio source"),
                                              tr("Each voice is emitted through it's own audio source"),
                                              false,
                                              true, false);

        p_audioX_ = createFloatParameter("vaudiox", tr("voice position x"),
                                         tr("X position where the voice is emitted in space"),
                                         0.0, 0.1);

        p_audioY_ = createFloatParameter("vaudioy", tr("voice position y"),
                                         tr("Y position where the voice is emitted in space"),
                                         0.0, 0.1);

        p_audioZ_ = createFloatParameter("vaudioz", tr("voice position z"),
                                         tr("Z position where the voice is emitted in space"),
                                         0.0, 0.1);

    endParameterGroup();
}

void Synthesizer::updateParameterVisibility()
{
    Object::updateParameterVisibility();

    synth_->updateParameterVisibility();

    const bool ispoly = p_polyAudio_->baseValue();
    p_audioX_->setVisible(ispoly);
    p_audioY_->setVisible(ispoly);
    p_audioZ_->setVisible(ispoly);
}

void Synthesizer::onParameterChanged(Parameter * p)
{
    Object::onParameterChanged(p);

    bool checkVoices = synth_->onParameterChanged(p);

    checkVoices |= (p == p_polyAudio_);

    if (checkVoices)
    {
        const bool ispoly = p_polyAudio_->baseValue();
        if (   (ispoly && synth_->synth()->numberVoices() != (uint)audios_.size())
            || (!ispoly && audios_.size() > 1))
            requestCreateAudioSources();
    }
}

void Synthesizer::onParametersLoaded()
{
    Object::onParametersLoaded();

    synth_->onParametersLoaded();
}

void Synthesizer::createAudioSources()
{
    Object::createAudioSources();

    const bool ispoly = p_polyAudio_->baseValue();

    audios_ = createOrDeleteAudioSources("voice", ispoly? synth_->synth()->numberVoices() : 1);
}

void Synthesizer::setSampleRate(uint samplerate)
{
    Object::setSampleRate(samplerate);

    synth_->synth()->setSampleRate(samplerate);
}

void Synthesizer::setNumberThreads(uint num)
{
    Object::setNumberThreads(num);

    audioBuffers_.resize(num);
    audioPos_.resize(num);
}

void Synthesizer::setBufferSize(uint bufferSize, uint thread)
{
    Object::setBufferSize(bufferSize, thread);

    // copy pointers to audiobuffers
    audioBuffers_[thread].resize(audios_.size());
    for (int i=0; i<audios_.size(); ++i)
        audioBuffers_[thread][i] = audios_[i]->samples(thread);

    audioPos_[thread].resize(audios_.size());
}

void Synthesizer::updateAudioTransformations(Double, uint thread)
{
    const Mat4 trans = transformation(thread, 0);

    for (int i=0; i<audios_.size(); ++i)
    {
        audios_[i]->setTransformation(trans * audioPos_[MO_AUDIO_THREAD][i], thread, 0);
    }
}

void Synthesizer::updateAudioTransformations(Double, uint size, uint thread)
{
    if (!p_polyAudio_->baseValue())
    for (auto a : audios_)
    {
        // copy the block of transformations
        a->setTransformation(transformations(thread), thread);
    }
    else
    {
        for (uint j=0; j<size; ++j)
        {
            const Mat4 trans = transformation(thread, j);

            for (int i=0; i<audios_.size(); ++i)
            {
                audios_[i]->setTransformation(trans * audioPos_[thread][i], thread, j);
            }
        }
    }
}

void Synthesizer::performAudioBlock(SamplePos pos, uint thread)
{
    const Double time = pos * sampleRateInv();

    if (!p_polyAudio_->baseValue())
    {
        // mono output
        synth_->feedSynth(time, thread);
        synth_->synth()->process(audios_[0]->samples(thread), bufferSize(thread));
    }
    else
    {
        // polyphonic output
        QVector<AUDIO::SynthVoice*> voices;
        synth_->feedSynth(time, thread, &voices);
        synth_->synth()->process(
                    &audioBuffers_[thread][0], bufferSize(thread));

        // place in space
        for (AUDIO::SynthVoice * v : voices)
        {
            // get info that SynthSetting attached to the voice
            auto data = static_cast<SynthSetting::VoiceData*>(v->userData());

            // set audio-source's transformation matrix
            audioPos_[thread][v->index()] = glm::translate(Mat4(1.f),
                        Vec3(
                            p_audioX_->value(data->timeStarted, thread),
                            p_audioY_->value(data->timeStarted, thread),
                            p_audioZ_->value(data->timeStarted, thread)
                        ));
        }
    }
}


} // namespace MO

