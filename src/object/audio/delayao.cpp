/** @file delayao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/


#include "delayao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/constants.h"
#include "audio/tool/delay.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(DelayAO)

class DelayAO::Private
{
    public:

    Private(DelayAO * ao)
        : ao            (ao)
        , curDelaySam   (0.f)
    { }

    void updateDelays();

    ParameterFloat
        * paramAmp,
        * paramTime,
        * paramMaxTime;
    ParameterInt
        * paramSamples;
        //* paramReset;
    //ParameterSelect
//        * paramInterpol;

    DelayAO * ao;
    std::vector<std::vector<AUDIO::AudioDelay>> delays;
    F32 curDelaySam;
};

DelayAO::DelayAO()
    : AudioObject   (),
      p_            (new Private(this))
{
    setName("Delay");
    setNumberChannelsAdjustable(true);
}

DelayAO::~DelayAO()
{
    delete p_;
}

QString DelayAO::infoString() const
{
    return tr("delay (samples) %1 / %2")
            .arg(p_->curDelaySam)
            .arg(p_->delays.empty()
                 ? 0 : p_->delays[0].empty() ? 0 : p_->delays[0][0].size());
}

void DelayAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aodly", 1);
}

void DelayAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aodly", 1);
}

void DelayAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("_delay_", tr("delay"));
    initParameterGroupExpanded("_delay_");

        p_->paramMaxTime = params()->createFloatParameter(
                    "_delay_max", tr("maximum time"),
                   tr("The maximum time delay for the delay line in seconds "
                      "(num. samples rounded to the next power of two)"),
                   2,
                   0.01, 60 * 60, /* one hour max */
                   0.1, true, false);

        p_->paramTime = params()->createFloatParameter(
                    "_delay_time", tr("delay time in seconds"),
                   tr("The current delay time in seconds"),
                    1.0, 0.05);
        p_->paramTime->setMinValue(0.);

        p_->paramSamples = params()->createIntParameter(
                    "_delay_samples", tr("delay time in samples"),
                    tr("Additional delaytime in samples, added to the seconds"),
                    0, true, true);
        p_->paramSamples->setMinValue(0);

        p_->paramAmp = params()->createFloatParameter(
                    "_delay_amp", tr("amplitude"),
                   tr("The output amplitude of the delayed signal"),
                    1.0, 0.05);

    params()->endParameterGroup();
}

void DelayAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    // nothing
}

void DelayAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    if (p == p_->paramMaxTime)
        p_->updateDelays();
}

void DelayAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->updateDelays();
}

void DelayAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->delays.resize(count);
    p_->updateDelays();
}

void DelayAO::setAudioBuffers(uint thread, uint ,
                             const QList<AUDIO::AudioBuffer*>& inputs,
                             const QList<AUDIO::AudioBuffer*>& outputs)
{
    size_t num = std::min(inputs.size(), outputs.size());
    p_->delays[thread].resize(num);
    p_->updateDelays();
}

void DelayAO::Private::updateDelays()
{
    for (auto& dv : delays)
        for (auto& d : dv)
            d.resize(paramMaxTime->baseValue() * ao->sampleRate());
}

void DelayAO::processAudio(const RenderTime& time)
{
    const bool isMod =
            p_->paramTime->isModulated()
         || p_->paramSamples->isModulated();

    if (!isMod) // parameter update per block
    {
        const F32
                amp = p_->paramAmp->value(time),
                samplesBack = p_->paramTime->value(time) * sampleRate()
                            + p_->paramSamples->value(time);
        p_->curDelaySam = samplesBack;

        AUDIO::AudioBuffer::process(
                    audioInputs(time.thread()), audioOutputs(time.thread()),
        [=](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            AUDIO::AudioDelay * delay = &p_->delays[time.thread()][chan];

            // write block into delay
            delay->writeBlock(in->readPointer(), in->blockSize());

            // put delayed block into out
            for (uint i = 0; i < out->blockSize(); ++i)
                out->write(i, amp * delay->read(in->blockSize() - i + samplesBack));
        });
    }
    // parameter update per sample
    else
    {
        AUDIO::AudioBuffer::process(
                    audioInputs(time.thread()), audioOutputs(time.thread()),
        [=](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            AUDIO::AudioDelay * delay = &p_->delays[time.thread()][chan];

            // write block into delay
            delay->writeBlock(in->readPointer(), in->blockSize());

            // put delayed block into out
            for (uint i = 0; i < out->blockSize(); ++i)
            {
                RenderTime btime(time);
                btime += SamplePos(i);

                const F32
                        amp = p_->paramAmp->value(btime),
                        samplesBack = p_->paramTime->value(btime) * sampleRate()
                                    + p_->paramSamples->value(btime);
                p_->curDelaySam = samplesBack;

                out->write(i, amp * delay->read(in->blockSize() - i + samplesBack));
            }
        });
    }
}

} // namespace MO
