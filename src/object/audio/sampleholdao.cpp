/** @file impulseao.cpp

    @brief A simple impulse generator

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "sampleholdao.h"

#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "audio/tool/audiobuffer.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(SampleHoldAO)

class SampleHoldAO::Private
{
    public:

    enum Mode {
        M_GATE,
        M_TRANSITION,
        M_NEG_TRANSITION
    };

    Private(SampleHoldAO * ao) : ao(ao) { }

    Mode mode() const { return (Mode)paramMode->baseValue(); }

    SampleHoldAO * ao;
    std::vector<Double> state, lastsample;
    ParameterFloat
            * paramTrigger;
    ParameterSelect
            * paramMode;
};

SampleHoldAO::SampleHoldAO(QObject *parent)
    : AudioObject (parent)
    , p_          (new Private(this))
{
    setName("S&H");
    setNumberAudioInputs(2);
    setNumberAudioOutputs(1);
}

SampleHoldAO::~SampleHoldAO()
{
    delete p_;
}

void SampleHoldAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aosamplehold", 1);
}

void SampleHoldAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aosamplehold", 1);
}

void SampleHoldAO::createParameters()
{
    AudioObject::createParameters();
    params()->beginParameterGroup("samplehold", tr("sample and hold"));

    p_->paramTrigger = params()->createFloatParameter("samplehold_trigger", tr("trigger"),
                                                      tr("The trigger input"),
                                                      0.0, 0.05);

    p_->paramMode    = params()->createSelectParameter("samplehold_mode", tr("sample and hold mode"),
                                                       tr("Selects the sample and hold mode"),
                                                       {"gate", "transition", "neg transition"},
                                                       {tr("Gate"), tr("Transition"), tr("Negative transition")},
                                                       {tr("When trigger is 0 the last value is repeated, otherwise the signal is passed through"),
                                                        tr("The signal is sampled on transion from 0 to a positiv value"),
                                                        tr("The signal is sampled on transition from a positive value to 0")},
                                                       {Private::M_GATE, Private::M_TRANSITION, Private::M_NEG_TRANSITION},
                                                       Private::M_GATE);
    params()->endParameterGroup();
}

void SampleHoldAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
    p_->state.resize(num);
    p_->lastsample.resize(num);
    for(uint i=0;i<num;++i) {
        p_->lastsample[i] = 0.0;
        p_->state[i]      = 0.0;
    }
}

QString SampleHoldAO::getAudioInputName(uint channel) const
{
    switch(channel)
    {
    case 0: return tr("in");
    case 1: return tr("trigger");
    }
    return AudioObject::getAudioInputName(channel);
}



void SampleHoldAO::processAudio(uint, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs  = audioInputs(thread),
            outputs = audioOutputs(thread);

    AUDIO::AudioBuffer
            * inTrig = inputs.size() < 2 ? 0 : inputs[1],
            * out = outputs.isEmpty() ? 0 : outputs[0];

    if(!out) return;

    switch(p_->mode()) {
    case Private::M_NEG_TRANSITION:
        for(uint i=0;i<out->blockSize();++i) {
            Double time = sampleRateInv() * (pos +i);
            Double trigger = p_->paramTrigger->value(time, thread);
            if(inTrig)
                trigger += inTrig->read(i);

            if(trigger <= 0.0 && p_->lastsample[thread] > 0.0)
                p_->state[thread] = inputs[0]->read(i);

            out->write(i, p_->state[thread]);
            p_->lastsample[thread] = trigger;
        }
        break;
    case Private::M_TRANSITION:
        for(uint i=0;i<out->blockSize();++i) {
            Double time = sampleRateInv() * (pos +i);
            Double trigger = p_->paramTrigger->value(time, thread);
            if(inTrig)
                trigger += inTrig->read(i);

            if(trigger > 0.0 && p_->lastsample[thread] <= 0.0)
                p_->state[thread] = inputs[0]->read(i);

            out->write(i, p_->state[thread]);
            p_->lastsample[thread] = trigger;
        }
        break;
    case Private::M_GATE:
    default:
        for(uint i=0;i<out->blockSize();++i) {
            Double time = sampleRateInv() * (pos +i);
            Double trigger = p_->paramTrigger->value(time, thread);
            if(inTrig)
                trigger += inTrig->read(i);

            if(trigger > 0)
                p_->state[thread] = inputs[0]->read(i);

            out->write(i, p_->state[thread]);
        }
    }
}
}
