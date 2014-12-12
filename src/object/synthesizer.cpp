/** @file synthesizer.cpp

    @brief Basic, non-graphic synthesizer object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#if 0

#include "synthesizer.h"
#include "io/datastream.h"
#include "audio/audiosource.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "param/parameterint.h"
#include "param/parameterselect.h"
#include "param/parametertext.h"
#include "io/log.h"
#include "audio/tool/synth.h"
#include "util/synthsetting.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"

namespace MO {

Synthesizer::VoiceEqu_::VoiceEqu_()
    : parserX   (new PPP_NAMESPACE::Parser()),
      parserY   (new PPP_NAMESPACE::Parser()),
      parserZ   (new PPP_NAMESPACE::Parser())
{
    initParser_(parserX);
    initParser_(parserY);
    initParser_(parserZ);
}

Synthesizer::VoiceEqu_::~VoiceEqu_()
{
    delete parserZ;
    delete parserY;
    delete parserX;
}

void Synthesizer::VoiceEqu_::initParser_(PPP_NAMESPACE::Parser * parser)
{
    parser->variables().add("time", &time,
            tr("Time when the voice was started in seconds").toStdString());
    parser->variables().add("timer", &timer,
            tr("Time when the voice was started in radians").toStdString());
    parser->variables().add("note", &note,
            tr("The value of the voice's note").toStdString());
    parser->variables().add("vel", &vel,
            tr("The velocity of the voice, typical range [0,1]").toStdString());
    parser->variables().add("freq", &freq,
            tr("The frequency of the voice in Hertz").toStdString());
    parser->variables().add("x", &x,
            tr("The current x position of the voice").toStdString());
    parser->variables().add("y", &x,
            tr("The current y position of the voice").toStdString());
    parser->variables().add("z", &x,
            tr("The current z position of the voice").toStdString());
}

void Synthesizer::VoiceEqu_::feedParser_(Double x, Double y, Double z, const AUDIO::SynthVoice &v)
{
    this->x = x;
    this->y = y;
    this->z = z;
    time = static_cast<SynthSetting::VoiceData*>(v.userData())->timeStarted;
    timer = time * TWO_PI;
    note = v.note();
    vel = v.velocity();
    freq = v.frequency();
}

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

    params()->beginParameterGroup("synthesizer", tr("synthesizer"));

        synth_->createParameters("");

    params()->endParameterGroup();

    params()->beginParameterGroup("audiosources", tr("audio sources"));

        p_polyAudio_ = params()->createBooleanParameter("polyaudiosrc", tr("polyphonic audio sources"),
                                              tr("When enabled, each synthesizer voice will be an audio source"),
                                              tr("All voices are emitted through one audio source"),
                                              tr("Each voice is emitted through it's own audio source"),
                                              false,
                                              true, false);

        p_audioX_ = params()->createFloatParameter("vaudiox", tr("voice position x"),
                                         tr("X position where the voice is emitted in space"),
                                         0.0, 0.1);

        p_audioY_ = params()->createFloatParameter("vaudioy", tr("voice position y"),
                                         tr("Y position where the voice is emitted in space"),
                                         0.0, 0.1);

        p_audioZ_ = params()->createFloatParameter("vaudioz", tr("voice position z"),
                                         tr("Z position where the voice is emitted in space"),
                                         0.0, 0.1);

        p_posEqu_ = params()->createBooleanParameter("doposequ", tr("position equation"),
                                           tr("Enables the position to be modified by equations"),
                                           tr("Off"),
                                           tr("On"),
                                           false,
                                           true, false);

        VoiceEqu_ tmp;
        p_equX_ = params()->createTextParameter("posequx", tr("x equation"),
                                      tr("Modifies the x position of where the voice is emitted"),
                                      TT_EQUATION, "0", true, false);
        p_equX_->setVariableDescriptions(tmp.parserX->variables().variableDescriptions());
        p_equX_->setVariableNames(tmp.parserX->variables().variableNames());

        p_equY_ = params()->createTextParameter("posequy", tr("y equation"),
                                      tr("Modifies the y position of where the voice is emitted"),
                                      TT_EQUATION, "0", true, false);
        p_equY_->setVariableDescriptions(tmp.parserX->variables().variableDescriptions());
        p_equY_->setVariableNames(tmp.parserX->variables().variableNames());

        p_equZ_ = params()->createTextParameter("posequz", tr("z equation"),
                                      tr("Modifies the z position of where the voice is emitted"),
                                      TT_EQUATION, "0", true, false);
        p_equZ_->setVariableDescriptions(tmp.parserX->variables().variableDescriptions());
        p_equZ_->setVariableNames(tmp.parserX->variables().variableNames());

    params()->endParameterGroup();
}

void Synthesizer::updateParameterVisibility()
{
    Object::updateParameterVisibility();

    synth_->updateParameterVisibility();

    const bool ispoly = p_polyAudio_->baseValue();
    p_audioX_->setVisible(ispoly);
    p_audioY_->setVisible(ispoly);
    p_audioZ_->setVisible(ispoly);

    const bool isequ = p_posEqu_->baseValue();
    p_equX_->setVisible(isequ);
    p_equY_->setVisible(isequ);
    p_equZ_->setVisible(isequ);
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

        setCallbacks_();
    }

    if (p == p_posEqu_
        || p == p_equX_
        || p == p_equY_
        || p == p_equZ_)
        updatePosParser_();
}

void Synthesizer::onParametersLoaded()
{
    Object::onParametersLoaded();

    synth_->onParametersLoaded();

    updatePosParser_();
    setCallbacks_();
}

void Synthesizer::updatePosParser_()
{
    if (!p_posEqu_->baseValue())
        return;

    for (VoiceEqu_ & v : voiceEqu_)
    {
        v.parserX->parse(p_equX_->baseValue().toStdString());
        v.parserY->parse(p_equY_->baseValue().toStdString());
        v.parserZ->parse(p_equZ_->baseValue().toStdString());
    }
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
    audioPosFifo_.resize(num);
    voiceEqu_.resize(num);
    updatePosParser_();
}

void Synthesizer::setBufferSize(uint bufferSize, uint thread)
{
    Object::setBufferSize(bufferSize, thread);

    // copy pointers to audiobuffers
    audioBuffers_[thread].resize(audios_.size());
    for (int i=0; i<audios_.size(); ++i)
        audioBuffers_[thread][i] = audios_[i]->samples(thread);

    audioPos_[thread].resize(audios_.size());
    audioPosFifo_[thread].resize(audios_.size());
    for (auto & f : audioPosFifo_[thread])
        f.clear();
}

void Synthesizer::setCallbacks_()
{
    if (!p_polyAudio_->baseValue())
    {
        synth_->synth()->setVoiceStartedCallback(0);
        synth_->synth()->setVoiceEndedCallback(0);
    }
    else
    {
        synth_->synth()->setVoiceStartedCallback([=](AUDIO::SynthVoice * v)
        {
            // get info that SynthSetting attached to the voice
            auto data = static_cast<SynthSetting::VoiceData*>(v->userData());

            VoicePos_ p;

            // get position from parameters
            Vec3 pos(
                p_audioX_->value(data->timeStarted, data->thread),
                p_audioY_->value(data->timeStarted, data->thread),
                p_audioZ_->value(data->timeStarted, data->thread)
                );

            // add values from equation
            if (p_posEqu_->baseValue())
            {
                VoiceEqu_ & equ = voiceEqu_[data->thread];
                equ.feedParser_(pos[0], pos[1], pos[2], *v);
                if (equ.parserX->ok())
                    pos[0] += equ.parserX->eval();
                if (equ.parserY->ok())
                    pos[1] += equ.parserY->eval();
                if (equ.parserZ->ok())
                    pos[2] += equ.parserZ->eval();
            }

            // set audio-source's transformation matrix
            p.trans = glm::translate(Mat4(1.f), pos);

            p.sample = v->startSample();
            p.sceneTime = data->timeStarted;

            // add to fifo
            audioPosFifo_[data->thread][v->index()].push_back(p);
        });
    }
}

void Synthesizer::updateAudioTransformations(Double, uint thread)
{
    const Mat4 trans = transformation(thread, 0);

    for (int i=0; i<audios_.size(); ++i)
    {
        // XXX It is quite hacky to pull the audio-thread information here
        // because there might be more than one audio-thread in the future!
        audios_[i]->setTransformation(trans * audioPos_[MO_AUDIO_THREAD][i], thread, 0);
    }
}

void Synthesizer::updateAudioTransformations(Double sceneTime, uint size, uint thread)
{
    if (!p_polyAudio_->baseValue())
    for (auto a : audios_)
    {
        // copy the block of transformations
        a->setTransformation(transformations(thread), thread);
    }
    else
    {            
        for (int i=0; i<audios_.size(); ++i)
        {
            for (uint j=0; j<size; ++j)
            {
                Double time = sceneTime + sampleRateInv() * j;

                // get object's transformation
                const Mat4 trans = transformation(thread, j);

                // get next audio transformation from fifo buffer
                if (!audioPosFifo_[thread][i].empty()
                        && time >= audioPosFifo_[thread][i].front().sceneTime)
                {
                    /*MO_DEBUG("new transformation: s=" << j
                             << " ss=" << audioPosFifo_[thread][i].front().sample
                             << " t=" << audioPosFifo_[thread][i].front().sceneTime);
                    */
                    audioPos_[thread][i] = audioPosFifo_[thread][i].front().trans;
                    audioPosFifo_[thread][i].pop_front();
                }

                // current audiosource transformation
                const Mat4 atrans = audioPos_[thread][i];

                audios_[i]->setTransformation(trans * atrans, thread, j);
            }
        }
    }
}

void Synthesizer::performAudioBlock(SamplePos pos, uint thread)
{
    if (!p_polyAudio_->baseValue())
    {
        // mono output
        synth_->feedSynth(pos, thread, bufferSize(thread));
        // get synth output
        synth_->synth()->process(audios_[0]->samples(thread), bufferSize(thread));
    }
    else
    {
        // polyphonic output
        synth_->feedSynth(pos, thread, bufferSize(thread));
        // get polyphonic synth output
        synth_->synth()->process(
                    &audioBuffers_[thread][0], bufferSize(thread));
    }
}


} // namespace MO

#endif
