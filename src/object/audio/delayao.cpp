/** @file delayao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include <memory>

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

    Private(DelayAO * ao) : ao(ao) { }

    void updateDelays();

    ParameterFloat
        * paramAmp,
        * paramTime,
        * paramMaxTime;
        //* paramReset;
    //ParameterSelect
//        * paramInterpol;

    DelayAO * ao;
    std::vector<std::shared_ptr<AUDIO::AudioDelay>> delays;
};

DelayAO::DelayAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private(this))
{
    setName("Delay");
    setNumberChannelsAdjustable(true);
}

DelayAO::~DelayAO()
{
    delete p_;
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

        p_->paramMaxTime = params()->createFloatParameter("_delay_max", tr("maximum time"),
                                                   tr("The maximum time delay for the delay line in seconds "
                                                      "(num. samples rounded to the next power of two)"),
                                                   2,
                                                   0.01, 60 * 60, /* one hour max */
                                                   0.1, true, false);

        p_->paramTime = params()->createFloatParameter("_delay_time", tr("delay time in seconds"),
                                                   tr("The current delay time in seconds"),
                                                   1.0, 0.05);
        p_->paramAmp = params()->createFloatParameter("_delay_amp", tr("amplitude"),
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

    p_->delays.clear();
    for (uint i=0; i<count; ++i)
        p_->delays.push_back( std::shared_ptr<AUDIO::AudioDelay>(
                                   new AUDIO::AudioDelay() ));
    p_->updateDelays();
}

void DelayAO::Private::updateDelays()
{
    for (auto & dp : delays)
    {
        AUDIO::AudioDelay * d = dp.get();

        d->resize(paramMaxTime->baseValue() * ao->sampleRate());
    }
}

void DelayAO::processAudio(uint , SamplePos pos, uint thread)
{
    const Double time = sampleRateInv() * pos;

    // update Delay
    AUDIO::AudioDelay * delay = p_->delays[thread].get();

    if (1) // parameter update per block
    {
        const F32
                amp = p_->paramAmp->value(time, thread),
                samplesBack = p_->paramTime->value(time, thread) * sampleRate();

        AUDIO::AudioBuffer::process(audioInputs(thread), audioOutputs(thread),
        [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
        {
            // write block into delay
            delay->writeBlock(in->readPointer(), in->blockSize());

            // put delayed block into out
            for (uint i = 0; i < out->blockSize(); ++i)
                out->write(i, amp * delay->read(in->blockSize() - i + samplesBack));
        });
    }
}

} // namespace MO
