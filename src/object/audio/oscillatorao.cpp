/** @file oscillatorao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include "oscillatorao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "math/constants.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(OscillatorAO)

OscillatorAO::OscillatorAO(QObject *parent)
    : AudioObject   (parent),
      phase_        (0)
{
    setName("Oscillator");
    setNumberAudioOutputs(1);
}

void OscillatorAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aoosc", 1);
}

void OscillatorAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aoosc", 1);
}

void OscillatorAO::createParameters()
{
    params()->beginParameterGroup("osc", tr("oscillator"));

        paramOffset_ = params()->createFloatParameter("osc_offset", tr("offset"),
                                                   tr("The offset added to the oscillator output"),
                                                   0.0, 0.1);

        paramAmp_ = params()->createFloatParameter("osc_amp", tr("amplitude"),
                                                   tr("The amplitude of the oscillator output"),
                                                   1.0, 0.1);
        paramFreq_ = params()->createFloatParameter("osc_freq", tr("frequency"),
                                                   tr("The frequency of the oscillator in Hertz"),
                                                   100.0, 1.0);
        paramPhase_ = params()->createFloatParameter("osc_phase", tr("phase"),
                                                   tr("The phase modulation in units [-1,1]"),
                                                   0.0, 0.0625);
    params()->endParameterGroup();
}


void OscillatorAO::processAudio(const QList<AUDIO::AudioBuffer *> &inputs,
                                const QList<AUDIO::AudioBuffer *> &outputs,
                                uint , SamplePos pos, uint thread)
{
    AUDIO::AudioBuffer
            * out = outputs.isEmpty() ? 0 : outputs[0],
            * inPhase = inputs.isEmpty() ? 0 : inputs[0];
    if (!out)
        return;

    const Double freqFac = sampleRateInv() * TWO_PI;

    F32 * write = out->writePointer();
    for (uint i = 0; i < out->blockSize(); ++i, ++write)
    {
        Double time = sampleRateInv() * (pos + i);

        *write = paramOffset_->value(time, thread)
                    + paramAmp_->value(time, thread) * (
                        std::sin(phase_ + paramPhase_->value(time, thread))
                    );

        phase_ += freqFac * paramFreq_->value(time, thread);

        if (inPhase)
            phase_ += inPhase->read(i);

        if (phase_ > TWO_PI)
            phase_ -= TWO_PI*2;
        else if (phase_ < -TWO_PI)
            phase_ += TWO_PI*2;

    }
}

} // namespace MO
