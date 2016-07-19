/** @file envelopefollowerao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#include "EnvelopeFollowerAO.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/tool/EnvelopeFollower.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "io/DataStream.h"
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
        * paramTimeDown,
        * paramNormal,
        * paramThreshold,
        * paramAverage;

    // per thread / per channel
    std::vector<std::vector<AUDIO::EnvelopeFollower>> envs;
};



EnvelopeFollowerAO::EnvelopeFollowerAO()
    : AudioObject   (),
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
    initParameterGroupExpanded("out");

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

        p_->paramThreshold = params()->createFloatParameter("_envf_thresh", tr("input threshold"),
                                                   tr("The value of how much the input signal "
                                                      "must raise above the current average to cause a raise in the output"),
                                                   0.0, 0.025);
        p_->paramAverage = params()->createFloatParameter(
                    "_envf_timeav", tr("time average"),
       tr("The time in seconds to gather the average amplitude used for thresholding"),
       1.0, 0.05);

        p_->paramNormal = params()->createFloatParameter(
                    "_envf_normal", tr("normalize"),
                    tr("Normalization of input according to average [0,1]"),
                                                   0.0, 0.0, 1.0, 0.05);

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

void EnvelopeFollowerAO::setAudioBuffers(uint thread, uint /*bufferSize*/,
                               const QList<AUDIO::AudioBuffer *> &inputs,
                               const QList<AUDIO::AudioBuffer *> &/*outputs*/)
{
    // create envelope follower for each channel
    p_->envs[thread].resize(inputs.size());
}

void EnvelopeFollowerAO::processAudio(const RenderTime& time)
{
    F32 fadeIn = p_->paramTimeUp->value(time),
        fadeOut = p_->paramTimeDown->value(time),
        ampIn = p_->paramInAmp->value(time),
        ampOut = p_->paramOutAmp->value(time),
        norm = p_->paramNormal->value(time),
        thres = p_->paramThreshold->value(time),
        average = p_->paramAverage->value(time);

    AUDIO::AudioBuffer::process(audioInputs(time.thread()), audioOutputs(time.thread()),
    [=](uint channel, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        AUDIO::EnvelopeFollower * env = &p_->envs[time.thread()][channel];

        // update envelope follower settings
        if (   env->fadeIn() != fadeIn
            || env->fadeOut() != fadeOut
            || env->averageSpeed() != average
            || env->sampleRate() != sampleRate())
        {
            env->setSampleRate(sampleRate());
            env->setFadeIn(fadeIn);
            env->setFadeOut(fadeOut);
            env->setAverageSpeed(average);
            env->updateCoefficients();
        }
        env->setThreshold(thres);
        env->setInputAmplitude(ampIn);
        env->setOutputAmplitude(ampOut);
        env->setNormalization(norm);

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
