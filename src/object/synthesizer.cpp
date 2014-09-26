/** @file synthesizer.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#include "synthesizer.h"
#include "io/datastream.h"
#include "audio/audiosource.h"
#include "param/parameterfloat.h"
#include "param/parameterint.h"
#include "param/parameterselect.h"
#include "io/log.h"
#include "audio/tool/synth.h"
#include "audio/tool/floatgate.h"

namespace MO {

MO_REGISTER_OBJECT(Synthesizer)

Synthesizer::Synthesizer(QObject *parent)
    : Object    (parent),
      synth_    (new AUDIO::Synth()),
      gate_     (new AUDIO::FloatGate<Double>())
{
    setName("Synthesizer");
}

Synthesizer::~Synthesizer()
{
    delete synth_;
    delete gate_;
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

    beginParameterGroup("synthesizer", tr("synthesizer"));

        p_numVoices_ = createIntParameter("num_voices", tr("number voices"),
                                           tr("The number of polyphonic voices"),
                                           synth_->numberVoices(), 1, 512, 1, true, false);

        p_gate_ = createFloatParameter("gate", tr("gate"),
                                           tr("A positive value starts a new note - all parameters below take affect then"),
                                           0.0);

        p_note_ = createIntParameter("note", tr("note"),
                                     tr("The note that is triggered on gate"),
                                     48, true, true);

        p_baseFreq_ = createFloatParameter("base_freq", tr("base frequency"),
                                           tr("The frequency in Hertz of the lowest C note"),
                                           synth_->baseFrequency());

        p_notesPerOct_ = createFloatParameter("notes_oct", tr("notes per octave"),
                                              tr("The number of notes per one octave"),
                                              synth_->notesPerOctave(), 0.00001, 256.0, 1.0);

        p_attack_ =   createFloatParameter("attack", tr("attack"),
                                           tr("Attack time of envelope in seconds"),
                                           synth_->attack(), 0.0, 10000.0, 0.05);

        p_decay_ =    createFloatParameter("decay", tr("decay"),
                                           tr("Decay time of envelope in seconds"),
                                           synth_->decay(), 0.0, 10000.0, 0.05);

        p_sustain_ =  createFloatParameter("sustain", tr("sustain"),
                                           tr("Sustain level of envelope"),
                                           synth_->sustain(), 0.05);

        p_release_ =  createFloatParameter("release", tr("release"),
                                           tr("Release time of envelope in seconds"),
                                           synth_->release(), 0.0, 10000.0, 0.05);

        p_waveform_ = createSelectParameter("waveform", tr("oscillator type"),
                                        tr("Selects the type of the oscillator waveform"),
                                        AUDIO::Waveform::typeIds,
                                        AUDIO::Waveform::typeNames,
                                        AUDIO::Waveform::typeStatusTips,
                                        AUDIO::Waveform::typeList,
                                        AUDIO::Waveform::T_SINE, true, false);

        p_pulseWidth_ = createFloatParameter("pulsewidth", tr("pulse width"),
                   tr("Pulsewidth of the oscillator waveform - describes the width of the positive edge"),
                   0.5, AUDIO::Waveform::minPulseWidth(), AUDIO::Waveform::maxPulseWidth(), 0.05);

        // ---- filter -----

        p_filterType_ = createSelectParameter("filtertype", tr("filter type"),
                                      tr("Selectes the type of filter"),
                                      AUDIO::MultiFilter::filterTypeIds,
                                      AUDIO::MultiFilter::filterTypeNames,
                                      AUDIO::MultiFilter::filterTypeStatusTips,
                                      AUDIO::MultiFilter::filterTypeEnums,
                                      synth_->filterType(), true, false);

        p_filterOrder_ = createIntParameter("filterorder", tr("filter order"),
                                     tr("The order (sharpness) of the filter for the 'nth order' types"),
                                     synth_->filterOrder(),
                                     1, 20,
                                     1);

        p_filterFreq_ = createFloatParameter("filterfreq", tr("filter frequency"),
                                     tr("Controls the filter frequency in Hertz"),
                                     synth_->filterFrequency(), 0.00001, 100000.0, 10.);


        p_filterReso_ = createFloatParameter("filterreso", tr("filter resonance"),
                                     tr("Controls the filter resonance - how much of the filter is fed-back to itself"),
                                     synth_->filterResonance(), 0.0, 1.0, 0.01);

        p_filterKeyFollow_ = createFloatParameter("filterkeyf", tr("filter key follow"),
                                     tr("Factor for voice frequency to be added to the filter frequency."),
                                     synth_->filterKeyFollower(), 0.01);

        p_filterEnv_ = createFloatParameter("filterenv", tr("filter envelope"),
                                     tr("Frequency of filter envelope in Hertz"),
                                     synth_->filterEnvelopeAmount(), 10.0);

        p_filterEnvKeyFollow_ = createFloatParameter("filterenvkeyf", tr("filter envelope key f."),
                                     tr("Factor for voice frequency to be added to the filter envelope amount."),
                                     synth_->filterEnvelopeKeyFollower(), 0.01);

        p_fattack_ =  createFloatParameter("fattack", tr("filter attack"),
                                           tr("Attack time of filter envelope in seconds"),
                                           synth_->filterAttack(), 0.0, 10000.0, 0.05);

        p_fdecay_ =   createFloatParameter("fdecay", tr("filter decay"),
                                           tr("Decay time of filter envelope in seconds"),
                                           synth_->filterDecay(), 0.0, 10000.0, 0.05);

        p_fsustain_ = createFloatParameter("fsustain", tr("filter sustain"),
                                           tr("Sustain level of filter envelope"),
                                           synth_->filterSustain(), 0.05);

        p_frelease_ = createFloatParameter("frelease", tr("frelease"),
                                           tr("Release time of filter envelope in seconds"),
                                           synth_->filterRelease(), 0.0, 10000.0, 0.05);

    endParameterGroup();
}

void Synthesizer::updateParameterVisibility()
{
    const auto wavetype = (AUDIO::Waveform::Type)p_waveform_->baseValue();
    const auto filtertype = (AUDIO::MultiFilter::FilterType)p_filterType_->baseValue();
    const bool
            isfilter = filtertype != AUDIO::MultiFilter::T_BYPASS;

    p_pulseWidth_->setVisible(
                AUDIO::Waveform::supportsPulseWidth(wavetype) );
    p_filterOrder_->setVisible(isfilter &&
                AUDIO::MultiFilter::supportsOrder(filtertype));
    p_filterFreq_->setVisible(isfilter);
    p_filterReso_->setVisible(isfilter);
    p_filterKeyFollow_->setVisible(isfilter);
    p_filterEnv_->setVisible(isfilter);
    p_filterEnvKeyFollow_->setVisible(isfilter);
    p_fattack_->setVisible(isfilter);
    p_fdecay_->setVisible(isfilter);
    p_fsustain_->setVisible(isfilter);
    p_frelease_->setVisible(isfilter);
}

void Synthesizer::onParameterChanged(Parameter * p)
{
    Object::onParameterChanged(p);

    if (p == p_numVoices_)
        synth_->setNumberVoices(p_numVoices_->baseValue());
}

void Synthesizer::onParametersLoaded()
{
    Object::onParametersLoaded();

    synth_->setNumberVoices(p_numVoices_->baseValue());
}

void Synthesizer::createAudioSources()
{
    Object::createAudioSources();

    audio_ = createAudioSource();
}

void Synthesizer::setSampleRate(uint samplerate)
{
    Object::setSampleRate(samplerate);
    synth_->setSampleRate(samplerate);
}

void Synthesizer::updateAudioTransformations(Double, uint thread)
{
    audio_->setTransformation(transformation(thread, 0), thread, 0);
}

void Synthesizer::updateAudioTransformations(Double, uint , uint thread)
{
    // copy the block of transformations
    audio_->setTransformation(transformations(thread), thread);
}

void Synthesizer::performAudioBlock(SamplePos pos, uint thread)
{
    const Double time = pos * sampleRateInv();

    Double gate = gate_->input(p_gate_->value(time, thread));

    if (gate > 0)
    {
        // notesPerOctave setting needs recalculation
        // so only set when changed
        const Double notesOct = p_notesPerOct_->value(time, thread);
        if (notesOct != synth_->notesPerOctave())
            synth_->setNotesPerOctave(notesOct);

        synth_->setBaseFrequency(p_baseFreq_->value(time, thread));
        synth_->setAttack(p_attack_->value(time, thread));
        synth_->setDecay(p_decay_->value(time, thread));
        synth_->setSustain(p_sustain_->value(time, thread));
        synth_->setRelease(p_release_->value(time, thread));
        synth_->setPulseWidth(p_pulseWidth_->value(time, thread) );
        synth_->setWaveform((AUDIO::Waveform::Type)p_waveform_->baseValue());
        synth_->setFilterType((AUDIO::MultiFilter::FilterType)p_filterType_->baseValue());
        synth_->setFilterOrder(p_filterOrder_->value(time, thread));
        synth_->setFilterFrequency(p_filterFreq_->value(time, thread));
        synth_->setFilterResonance(p_filterReso_->value(time, thread));
        synth_->setFilterKeyFollower(p_filterKeyFollow_->value(time, thread));
        synth_->setFilterEnvelopeAmount(p_filterEnv_->value(time, thread));
        synth_->setFilterEnvelopeKeyFollower(p_filterEnvKeyFollow_->value(time, thread));
        synth_->setFilterAttack(p_fattack_->value(time, thread));
        synth_->setFilterDecay(p_fdecay_->value(time, thread));
        synth_->setFilterSustain(p_fsustain_->value(time, thread));
        synth_->setFilterRelease(p_frelease_->value(time, thread));

        const int note = p_note_->value(time, thread);

        synth_->noteOn(note, gate);
    }

    synth_->process(audio_->getSamples(thread), bufferSize(thread));
}


} // namespace MO

