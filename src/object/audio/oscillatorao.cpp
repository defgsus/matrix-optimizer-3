/** @file oscillatorao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include "oscillatorao.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/wavetable.h"
#include "audio/tool/bandlimitwavetablegenerator.h"
#include "audio/tool/floatgate.h"
#include "math/constants.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(OscillatorAO)


class OscillatorAO::Private
{
    public:

    enum Mode
    {
        M_OSCILLATOR,
        M_EQUATION
    };

    Private(OscillatorAO * ao) : ao(ao) { }

    Mode mode() const { return (Mode)paramMode->baseValue(); }
    AUDIO::Waveform::Type oscType() const
        { return (AUDIO::Waveform::Type)paramOscType->baseValue(); }

    void updateWavetable();
    void processAudio(uint size, SamplePos pos, uint thread);

    OscillatorAO * ao;
    std::vector<Double> phase;
    ParameterFloat
        * paramFreq,
        * paramPhase,
        * paramAmp,
        * paramOffset,
        * paramPulseWidth,
        * paramSync;
        //* paramSync;
    ParameterSelect
        * paramMode,
        * paramOscType,
        * paramTableSize,
        * paramNormalize;
    ParameterText
        * paramEquation;

    AUDIO::Wavetable<F32> wtable;
    std::vector<AUDIO::FloatGate<Double>> gates;
};







OscillatorAO::OscillatorAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private(this))
{
    setName("Oscillator");

    setNumberAudioInputs(4);
    setNumberAudioOutputs(1);
}

OscillatorAO::~OscillatorAO()
{
    delete p_;
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

        p_->paramOffset = params()->createFloatParameter("osc_offset", tr("offset"),
                                                   tr("The offset added to the oscillator output"),
                                                   0.0, 0.01);

        p_->paramAmp = params()->createFloatParameter("osc_amp", tr("amplitude"),
                                                   tr("The amplitude of the oscillator output"),
                                                   1.0, 0.05);
        p_->paramFreq = params()->createFloatParameter("osc_freq", tr("frequency"),
                                                   tr("The frequency of the oscillator in Hertz"),
                                                   100.0, 1.0);
        p_->paramPhase = params()->createFloatParameter("osc_phase", tr("phase"),
                                                   tr("The phase modulation in units [-1,1]"),
                                                   0.0, 0.0625);
        p_->paramSync = params()->createFloatParameter("osc_sync", tr("hard sync"),
                                                   tr("A signal >= 0 resets the phase of the oscillator"),
                                                   0.0);

        p_->paramMode = params()->createSelectParameter("osc_mode", tr("oscillator mode"),
                                            tr("Selects the type of waveform"),
                                            { "osc", "equ" },
                                            { tr("oscillator"), tr("equation") },
                                            { tr("Basic oscillator waveforms"),
                                              tr("A waveform described by a mathematical formula") },
                                            { Private::M_OSCILLATOR, Private::M_EQUATION },
                                            Private::M_OSCILLATOR);

        p_->paramOscType = params()->createSelectParameter("osc_type", tr("waveform"),
                                                   tr("Selects the type of the oscillator waveform"),
                                                   AUDIO::Waveform::typeAudioIds,
                                                   AUDIO::Waveform::typeAudioNames,
                                                   AUDIO::Waveform::typeAudioStatusTips,
                                                   AUDIO::Waveform::typeAudioList,
                                                   AUDIO::Waveform::T_SINE, true, false);

        p_->paramPulseWidth = params()->createFloatParameter("osc_pulsewidth", tr("pulse width"),
                                                  tr("Pulsewidth of the waveform, describes the width of the positive edge"),
                                                  0.5, AUDIO::Waveform::minPulseWidth(), AUDIO::Waveform::maxPulseWidth(), 0.05,
                                                  true, false);

        p_->paramEquation = params()->createTextParameter("osc_equ", tr("wavetable equation"),
                  tr("The equation is interpreted as a function of time and should be periodic "
                     "in the range [0,1]"),
                  TT_EQUATION,
                  "sin(xr)", true, false);
        p_->paramEquation->setVariableNames(QStringList() << "x" << "xr");
        p_->paramEquation->setVariableDescriptions(QStringList()
                  << tr("wavetable second [0,1]") << tr("radians of wavetable second [0,TWO_PI]"));

        p_->paramTableSize = params()->createSelectParameter("osc_table_size", tr("wavetable size"),
                    tr("Number of samples in the wavetable"),
                    { "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" },
                    { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
                    { "", "", "", "", "", "", "", "", "", "", "", "", "" },
                    { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
                    4096, true, false);

        p_->paramNormalize = params()->createBooleanParameter("osc_norm_wt", tr("normalize wavetable"),
                                          tr("Selects whether the wavetable should be normalized to [-1,1]"),
                                          tr("No normalization"),
                                          tr("Normalization on"),
                                          false, true, false);

    params()->endParameterGroup();
}

void OscillatorAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
    p_->updateWavetable();
}

void OscillatorAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);
    if (p == p_->paramTableSize
        || p == p_->paramMode
        || p == p_->paramOscType
        || p == p_->paramPulseWidth
        || p == p_->paramEquation
        || p == p_->paramNormalize)
        p_->updateWavetable();
}

void OscillatorAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    const bool
            isosc = p_->mode() == Private::M_OSCILLATOR,
            isequ = p_->mode() == Private::M_EQUATION,
            ispw = isosc && AUDIO::Waveform::supportsPulseWidth(p_->oscType());

    p_->paramOscType->setVisible(isosc);
    p_->paramPulseWidth->setVisible(isosc);
    p_->paramEquation->setVisible(isequ);
    p_->paramPulseWidth->setVisible(ispw);
}

void OscillatorAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);

    p_->gates.resize(num);
    p_->phase.resize(num);
    for (auto & f : p_->phase)
        f = 0.0;
}

QString OscillatorAO::getAudioInputName(uint channel) const
{
    switch (channel)
    {
        case 0: return tr("offset");
        case 1: return tr("amplitude");
        case 2: return tr("frequency");
        case 3: return tr("phase");
    }
    return AudioObject::getAudioInputName(channel);
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

    F32 * write = out->writePointer();

    if (inputs.isEmpty())
    for (uint i = 0; i < out->blockSize(); ++i, ++write)
    {
        // time for parameter reads
        Double time = sampleRateInv() * (pos + i);

        // update phase
        p_->phase[thread] += sampleRateInv() * p_->paramFreq->value(time, thread);

        // keep in bounds
        if (p_->phase[thread] > 1)
            p_->phase[thread] -= 2;
        else
        if (p_->phase[thread] < -1)
            p_->phase[thread] += 2;

        // check for sync
        if (p_->gates[thread].input(p_->paramSync->value(time, thread)))
            p_->phase[thread] = 0.0;

        // get sample
        *write = p_->paramOffset->value(time, thread)
                    + p_->paramAmp->value(time, thread) * (
                        p_->wtable.value(
                                p_->phase[thread] + p_->paramPhase->value(time, thread) )
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

            // read parameters and add audio signal
            Double ofs = p_->paramOffset->value(time, thread);
            if (inOfs)
                ofs += inOfs->read(i);
            Double amp = p_->paramAmp->value(time, thread);
            if (inAmp)
                amp += inAmp->read(i);
            Double freq = p_->paramFreq->value(time, thread);
            if (inFreq)
                freq += inFreq->read(i);
            Double phase = p_->paramPhase->value(time, thread);
            if (inPhase)
                phase += inPhase->read(i);

            // advance phase
            p_->phase[thread] += sampleRateInv() * freq;

            // keep in bounds
            if (p_->phase[thread] > 1)
                p_->phase[thread] -= 2;
            else
            if (p_->phase[thread] < -1)
                p_->phase[thread] += 2;

            // check for sync
            if (p_->gates[thread].input(p_->paramSync->value(time, thread)))
                p_->phase[thread] = 0.0;

            // get wavetable at phase
            *write = ofs + amp * (
                            p_->wtable.value( p_->phase[thread] + phase )
                        );

        }
    }
}





void OscillatorAO::Private::updateWavetable()
{
    AUDIO::BandlimitWavetableGenerator gen;
    gen.setTableSize(paramTableSize->baseValue());
    gen.setWaveform(oscType());
    gen.setPulseWidth(paramPulseWidth->baseValue());
    gen.setMode(mode() == M_OSCILLATOR ? AUDIO::BandlimitWavetableGenerator::M_WAVEFORM
                                       : AUDIO::BandlimitWavetableGenerator::M_EQUATION);
    gen.setEquation(paramEquation->baseValue());
    gen.createWavetable(wtable);

    if (paramNormalize->baseValue())
        wtable.normalize();
}















} // namespace MO
