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

    setNumberAudioInputs(4);
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
    AudioObject::createParameters();

    params()->beginParameterGroup("osc", tr("oscillator"));

        paramOffset_ = params()->createFloatParameter("osc_offset", tr("offset"),
                                                   tr("The offset added to the oscillator output"),
                                                   0.0, 0.01);

        paramAmp_ = params()->createFloatParameter("osc_amp", tr("amplitude"),
                                                   tr("The amplitude of the oscillator output"),
                                                   1.0, 0.01);
        paramFreq_ = params()->createFloatParameter("osc_freq", tr("frequency"),
                                                   tr("The frequency of the oscillator in Hertz"),
                                                   100.0, 1.0);
        paramPhase_ = params()->createFloatParameter("osc_phase", tr("phase"),
                                                   tr("The phase modulation in units [-1,1]"),
                                                   0.0, 0.0625);
    params()->endParameterGroup();
}

void OscillatorAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);

    phase_.resize(num);
    for (auto & f : phase_)
        f = 0.0;
}

QString OscillatorAO::getInputName(uint channel) const
{
    switch (channel)
    {
        case 0: return tr("offset");
        case 1: return tr("amplitude");
        case 2: return tr("frequency");
        case 3: return tr("phase");
    }
    return AudioObject::getInputName(channel);
}

void OscillatorAO::processAudio(uint , SamplePos pos, uint thread)
{
    const QList<AUDIO::AudioBuffer*>&
            inputs = audioInputs(thread),
            outputs = audioOutputs(thread);

    AUDIO::AudioBuffer
            * out = outputs.isEmpty() ? 0 : outputs[0];
    if (!out)
        return;

    const Double freqFac = sampleRateInv() * TWO_PI;

    F32 * write = out->writePointer();

    if (inputs.isEmpty())
    for (uint i = 0; i < out->blockSize(); ++i, ++write)
    {
        Double time = sampleRateInv() * (pos + i);

        phase_[thread] += freqFac * paramFreq_->value(time, thread);

        if (phase_[thread] > TWO_PI)
            phase_[thread] -= TWO_PI*2;
        else
        if (phase_[thread] < -TWO_PI)
            phase_[thread] += TWO_PI*2;

        *write = paramOffset_->value(time, thread)
                    + paramAmp_->value(time, thread) * (
                        std::sin(phase_[thread] + paramPhase_->value(time, thread))
                    );
    }

    // version with audio input modulation
    else
    {
        AUDIO::AudioBuffer
            * inOfs = inputs[0],
            * inAmp = inputs.size() < 2 ? 0 : inputs[1],
            * inFreq = inputs.size() < 3 ? 0 : inputs[2],
            * inPhase = inputs.size() < 4 ? 0 : inputs[3];

        for (uint i = 0; i < out->blockSize(); ++i, ++write)
        {
            Double time = sampleRateInv() * (pos + i);

            Double ofs = paramOffset_->value(time, thread);
            if (inOfs)
                ofs += inOfs->read(i);
            Double amp = paramAmp_->value(time, thread);
            if (inAmp)
                amp += inAmp->read(i);
            Double freq = paramFreq_->value(time, thread);
            if (inFreq)
                freq += inFreq->read(i);

            phase_[thread] += freqFac * freq;

            if (inPhase)
                phase_[thread] += inPhase->read(i);

            if (phase_[thread] > TWO_PI)
                phase_[thread] -= TWO_PI*2;
            else
            if (phase_[thread] < -TWO_PI)
                phase_[thread] += TWO_PI*2;

            *write = ofs + amp * (
                            std::sin(phase_[thread] + paramPhase_->value(time, thread))
                        );

        }
    }
}

} // namespace MO
