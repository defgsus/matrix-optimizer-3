/** @file impulseao.cpp

    @brief A simple impulse generator

    <p>(c) 2014, martin.huenniger@yahoo.de</p>
    <p>All rights reserved</p>

    <p>created 15.12.2014</p>
*/

#include "impulseao.h"

#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "audio/tool/audiobuffer.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(ImpulseAO)

class ImpulseAO::Private
{
    public:

    enum Mode {
        M_OSCILLATOR //,
        //M_GENERATOR
    };

    Private(ImpulseAO *ao) : ao(ao) { }

    Mode mode() const { return (Mode)paramMode->baseValue(); }

    void processAudio(uint size, SamplePos pos, uint thread );

    ImpulseAO * ao;
    std::vector<Double> phase;
    ParameterFloat
            * paramAmp,
            * paramFreq,
            * paramPhase;
    ParameterSelect
            * paramMode;
};

ImpulseAO::ImpulseAO(QObject *parent)
    : AudioObject (parent)
    , p_          (new Private(this))
{
    setName("Impulse");
    setNumberAudioInputs(3);
    setNumberAudioOutputs(1);
}

ImpulseAO::~ImpulseAO()
{
    delete p_;
}

void ImpulseAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aoimpulse", 1);
}

void ImpulseAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aoimpulse",1);
}

void ImpulseAO::createParameters()
{
    AudioObject::createParameters();
    params()->beginParameterGroup("impulse", tr("impulse-generator"));
    initParameterGroupExpanded("impulse");

        p_->paramAmp  = params()->createFloatParameter("impulse_amp", tr("amplitude"),
                                                       tr("The Amplitude of the impulses"),
                                                       1.0, 0.05);
        p_->paramFreq = params()->createFloatParameter("impulse_freq", tr("frequency"),
                                                       tr("The Frequency of the impulses in Hz"),
                                                       100.0, 1.0);
        p_->paramPhase = params()->createFloatParameter("impulse_phase", tr("phase"),
                                                       tr("The phase modulation in units [-1,1]"),
                                                       0.0, 0.0625);
    params()->endParameterGroup();
}

void ImpulseAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
}

void ImpulseAO::onParameterChanged(Parameter *p)
{
    AudioObject::onParameterChanged(p);
}

void ImpulseAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();
}

void ImpulseAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);
    p_->phase.resize(num);
    for(auto & f : p_->phase)
        f = 0.0;
}

QString ImpulseAO::getAudioInputName(uint channel) const
{
    switch(channel)
    {
        case 0: return tr("amplitude");
        case 1: return tr("frequency");
        case 2: return tr("phase");
    }
    return AudioObject::getAudioInputName(channel);
}

void ImpulseAO::processAudio(uint, SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs  = audioInputs(thread),
            outputs = audioOutputs(thread);

    AUDIO::AudioBuffer
            * out = outputs.isEmpty() ? 0 : outputs[0];

    if(!out) return;

    F32 * write = out->writePointer();

    if(inputs.isEmpty()) {
        for(uint i=0;i<out->blockSize();++i,++write) {
            // time for parameter reads
            Double time = sampleRateInv() * (pos + i);

            // update phase
            p_->phase[thread] += sampleRateInv() * p_->paramFreq->value(time, thread);

            *write = 0.0;

            // keep in bounds
            if (p_->phase[thread] > 1)
            {
                p_->phase[thread] -= 2;
                *write = 1.0 * p_->paramAmp->value(time,thread);
            }
            else
            if (p_->phase[thread] < -1) {
                p_->phase[thread] += 2;
                *write = 1.0 * p_->paramAmp->value(time,thread);
            }
        }
    } else {
        // audio input modulation
        AUDIO::AudioBuffer
                * inAmp   = inputs.size() < 2 ? 0 : inputs[1],
                * inFreq  = inputs.size() < 3 ? 0 : inputs[2],
                * inPhase = inputs.size() < 4 ? 0 : inputs[3];
        for(uint i=0;i<out->blockSize(); ++i, ++write) {
            Double time = sampleRateInv() * (pos + i);

            // read parameters and add audio signal
            Double amp = p_->paramAmp->value(time,thread);
            if(inAmp)
                amp += inAmp->read(i);
            Double freq = p_->paramFreq->value(time,thread);
            if(inFreq)
                freq += inFreq->read(i);
            Double phase = p_->paramPhase->value(time,thread);
            if(inPhase)
                phase += inPhase->read(i);

            p_->phase[thread] += sampleRateInv() * freq;

            *write = 0.0;

            // keep in bounds
            if (p_->phase[thread] > 1)
            {
                p_->phase[thread] -= 2;
                *write = 1.0 * amp;
            }
            else
            if (p_->phase[thread] < -1) {
                p_->phase[thread] += 2;
                *write = 1.0 * amp;
            }
        }
    }

}

}
