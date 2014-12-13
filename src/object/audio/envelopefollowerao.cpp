/** @file envelopefollowerao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#include "envelopefollowerao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/envelopefollower.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/modulatoroutput.h"
#include "io/datastream.h"
#include "io/error.h"


namespace MO {

MO_REGISTER_OBJECT(EnvelopeFollowerAO)

class EnvelopeFollowerAO::Private
{
    public:

    Private(EnvelopeFollowerAO * ao) : ao(ao) { }

    EnvelopeFollowerAO * ao;

    ParameterFloat
        * paramInAmp,
        * paramOutAmp,
        * paramTimeUp,
        * paramTimeDown;

    // per thread / per channel
    std::vector<std::vector<AUDIO::EnvelopeFollower>> envs;
};



EnvelopeFollowerAO::EnvelopeFollowerAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private(this))
{
    setName("EnvelopeFollower");
    setNumberChannelsAdjustable(true);
}

EnvelopeFollowerAO::~EnvelopeFollowerAO()
{
    delete p_;
}

void EnvelopeFollowerAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aoenvf", 1);
}

void EnvelopeFollowerAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aoenvf", 1);
}

void EnvelopeFollowerAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("out", tr("output"));

        p_->paramInAmp = params()->createFloatParameter("_envf_inamp", tr("input amplitude"),
                                                   tr("The amplitude of the audio input"),
                                                   1.0, 0.05);
        p_->paramOutAmp = params()->createFloatParameter("_envf_outamp", tr("output amplitude"),
                                                   tr("The amplitude of the envelope output"),
                                                   1.0, 0.05);
        p_->paramTimeUp = params()->createFloatParameter("_envf_timeup", tr("time up"),
                                                   tr("The time in seconds to follow a rising signal volume"),
                                                   .005, 0.005);
        p_->paramTimeDown = params()->createFloatParameter("_envf_timedown", tr("time down"),
                                                   tr("The time in seconds to follow a falling signal volume"),
                                                   .5, 0.05);
    params()->endParameterGroup();
}

void EnvelopeFollowerAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
    createEnvOuts_();
}

void EnvelopeFollowerAO::onParameterChanged(Parameter *p)
{
    AudioObject::onParameterChanged(p);
    createEnvOuts_();
}

void EnvelopeFollowerAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);

    p_->envs.resize(num);
}

void EnvelopeFollowerAO::createEnvOuts_()
{
    /* XXX ModulatorOutputs not finished yet
     *
    // create modulator output for each channel
    QList<ModulatorOutput*> mods;
    for (uint i = 0; i < numAudioOutputs(); ++i)
        mods.append( new ModulatorOutputStaticFloat(this, QString::number(i), 0.0) );
    // pass to Object
    setModulatorOutputs(mods);
    */
}

void EnvelopeFollowerAO::setAudioBuffers(uint thread,
                               const QList<AUDIO::AudioBuffer *> &inputs,
                               const QList<AUDIO::AudioBuffer *> &/*outputs*/)
{
    // create envelope follower for each channel
    p_->envs[thread].resize(inputs.size());
}

void EnvelopeFollowerAO::processAudio(uint , SamplePos pos, uint thread)
{
    const Double time = sampleRateInv() * pos;

    F32 fadeIn = p_->paramTimeUp->value(time, thread),
        fadeOut = p_->paramTimeDown->value(time, thread),
        ampIn = p_->paramInAmp->value(time, thread),
        ampOut = p_->paramOutAmp->value(time, thread);

    AUDIO::AudioBuffer::process(audioInputs(thread), audioOutputs(thread),
    [=](uint channel, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        AUDIO::EnvelopeFollower * env = &p_->envs[thread][channel];

        // update envelope follower settings
        if (   env->fadeIn() != fadeIn
            || env->fadeOut() != fadeOut
            || env->sampleRate() != sampleRate())
        {
            env->setSampleRate(sampleRate());
            env->setFadeIn(fadeIn);
            env->setFadeOut(fadeOut);
            env->updateCoefficients();
        }
        env->setInputAmplitude(ampIn);
        env->setOutputAmplitude(ampOut);

        // process a block
        //F32 val =
                env->process(in->readPointer(), out->writePointer(), in->blockSize());

        /*
        // write into output
        MO_ASSERT(modulatorOutputs().size() > (int)channel, "modouts not uptodate");
        static_cast<ModulatorOutputStaticFloat*>(
                    modulatorOutputs()[channel])->setValue(val);
        */
    });
}


} // namespace MO
