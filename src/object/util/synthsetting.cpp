/** @file synthsetting.cpp

    @brief Wrapper for AUDIO::Synth and it's parameters for an Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 28.09.2014</p>
*/

#include <QVector>

#include "synthsetting.h"
#include "object/object.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfloat.h"
#include "audio/tool/synth.h"
#include "audio/tool/floatgate.h"

namespace MO {


SynthSetting::SynthSetting(Object *parent)
    : QObject   (parent),
      o_        (parent),
      synth_    (new AUDIO::Synth()),
      gate_     (new AUDIO::FloatGate<Double>())
{
}

SynthSetting::~SynthSetting()
{
    delete gate_;
    delete synth_;
}

void SynthSetting::createParameters(const QString &id_suffix)
{
    p_numVoices_ = o_->createIntParameter("num_voices" + id_suffix, tr("number voices"),
                                       tr("The number of polyphonic voices"),
                                       synth_->numberVoices(), 1, 512, 1, true, false);

    p_policy_ = o_->createSelectParameter("voice_policy", tr("voice policy"),
                                          tr("Sets the policy to use when the maximum polyphony "
                                             "is reached and a new note-on is requested"),
                                          AUDIO::Synth::voicePolicyIds,
                                          AUDIO::Synth::voicePolicyNames,
                                          AUDIO::Synth::voicePolicyStatusTips,
                                          AUDIO::Synth::voicePolicyEnums,
                                          (int)synth_->voicePolicy(), true, false);

    p_volume_ = o_->createFloatParameter("volume" + id_suffix, tr("volume"),
                                       tr("Master volume of all played voices"),
                                       synth_->volume(), 0.0, 100000.0, 0.1);

    p_gate_ = o_->createFloatParameter("gate" + id_suffix, tr("gate"),
                                       tr("A positive value starts a new note with the value as amplitude "
                                          "- all parameters below take affect then"),
                                       0.0);

    p_note_ = o_->createIntParameter("note" + id_suffix, tr("note"),
                                 tr("The note that is triggered on a positive input to gate"),
                                 48, true, true);

    p_numUnison_ = o_->createIntParameter("num_unison" + id_suffix, tr("number unisono voices"),
                                       tr("The number of unisono voices that will be played for one note"),
                                       synth_->unisonVoices(), 1, 512, 1, true, true);

    p_combinedUnison_ = o_->createBooleanParameter("comb_unison" + id_suffix, tr("combined unisono voices"),
                                       tr("Should unisono voices be individual synthesizer voices "
                                          "or should they be combined into one."),
                                       tr("Each unisono voice is a different synthesizer voice."),
                                       tr("All unisono voices are combined into one synthesizer voice."),
                                       synth_->combinedUnison(), true, false);

    p_unisonNoteStep_ = o_->createIntParameter("unison_notestep" + id_suffix, tr("unisono note step"),
                                       tr("Note to be added/subtracted for each unisono voice"),
                                       synth_->unisonNoteStep(), true, true);

    p_unisonDetune_ = o_->createFloatParameter("unison_detune" + id_suffix, tr("unisono detune"),
                                       tr("The amount of random detuning for each individual "
                                          "unisono voice in cents (100 per full note)"),
                                       synth_->unisonDetune());

    p_baseFreq_ = o_->createFloatParameter("base_freq" + id_suffix, tr("base frequency"),
                                       tr("The frequency in Hertz of the lowest C note"),
                                       synth_->baseFrequency());

    p_notesPerOct_ = o_->createFloatParameter("notes_oct" + id_suffix, tr("notes per octave"),
                                          tr("The number of notes per one octave"),
                                          synth_->notesPerOctave(), 0.00001, 256.0, 1.0);

    p_attack_ = o_->createFloatParameter("attack" + id_suffix, tr("attack"),
                                       tr("Attack time of envelope in seconds"),
                                       synth_->attack(), 0.0, 10000.0, 0.05);

    p_decay_ = o_->createFloatParameter("decay" + id_suffix, tr("decay"),
                                       tr("Decay time of envelope in seconds"),
                                       synth_->decay(), 0.0, 10000.0, 0.05);

    p_sustain_ = o_->createFloatParameter("sustain" + id_suffix, tr("sustain"),
                                       tr("Sustain level of envelope"),
                                       synth_->sustain(), 0.05);

    p_release_ = o_->createFloatParameter("release" + id_suffix, tr("release"),
                                       tr("Release time of envelope in seconds"),
                                       synth_->release(), 0.0, 10000.0, 0.05);

    p_waveform_ = o_->createSelectParameter("waveform" + id_suffix, tr("oscillator type"),
                                    tr("Selects the type of the oscillator waveform"),
                                    AUDIO::Waveform::typeIds,
                                    AUDIO::Waveform::typeNames,
                                    AUDIO::Waveform::typeStatusTips,
                                    AUDIO::Waveform::typeList,
                                    AUDIO::Waveform::T_SINE, true, false);

    p_pulseWidth_ = o_->createFloatParameter("pulsewidth" + id_suffix, tr("pulse width"),
               tr("Pulsewidth of the oscillator waveform - describes the width of the positive edge"),
               0.5, AUDIO::Waveform::minPulseWidth(), AUDIO::Waveform::maxPulseWidth(), 0.05);

    // ---- filter -----

    p_filterType_ = o_->createSelectParameter("filtertype" + id_suffix, tr("filter type"),
                                  tr("Selectes the type of filter"),
                                  AUDIO::MultiFilter::filterTypeIds,
                                  AUDIO::MultiFilter::filterTypeNames,
                                  AUDIO::MultiFilter::filterTypeStatusTips,
                                  AUDIO::MultiFilter::filterTypeEnums,
                                  synth_->filterType(), true, false);

    p_filterOrder_ = o_->createIntParameter("filterorder" + id_suffix, tr("filter order"),
                                 tr("The order (sharpness) of the filter for the 'nth order' types"),
                                 synth_->filterOrder(),
                                 1, 10,
                                 1);

    p_filterFreq_ = o_->createFloatParameter("filterfreq" + id_suffix, tr("filter frequency"),
                                 tr("Controls the filter frequency in Hertz"),
                                 synth_->filterFrequency(), 0.00001, 100000.0, 10.);


    p_filterReso_ = o_->createFloatParameter("filterreso" + id_suffix, tr("filter resonance"),
                                 tr("Controls the filter resonance or quality - how steep the transition between passband and stopband is"),
                                 synth_->filterResonance(), 0.0, 1.0, 0.01);

    p_filterKeyFollow_ = o_->createFloatParameter("filterkeyf" + id_suffix, tr("filter key follow"),
                                 tr("Factor for voice frequency to be added to the filter frequency."),
                                 synth_->filterKeyFollower(), 0.01);

    p_filterEnv_ = o_->createFloatParameter("filterenv" + id_suffix, tr("filter envelope"),
                                 tr("Frequency of filter envelope in Hertz"),
                                 synth_->filterEnvelopeAmount(), 10.0);

    p_filterEnvKeyFollow_ = o_->createFloatParameter("filterenvkeyf" + id_suffix, tr("filter envelope key f."),
                                 tr("Factor for voice frequency to be added to the filter envelope amount."),
                                 synth_->filterEnvelopeKeyFollower(), 0.01);

    p_fattack_ = o_->createFloatParameter("fattack" + id_suffix, tr("filter attack"),
                                       tr("Attack time of filter envelope in seconds"),
                                       synth_->filterAttack(), 0.0, 10000.0, 0.05);

    p_fdecay_ = o_->createFloatParameter("fdecay" + id_suffix, tr("filter decay"),
                                       tr("Decay time of filter envelope in seconds"),
                                       synth_->filterDecay(), 0.0, 10000.0, 0.05);

    p_fsustain_ = o_->createFloatParameter("fsustain" + id_suffix, tr("filter sustain"),
                                       tr("Sustain level of filter envelope"),
                                       synth_->filterSustain(), 0.05);

    p_frelease_ = o_->createFloatParameter("frelease" + id_suffix, tr("frelease"),
                                       tr("Release time of filter envelope in seconds"),
                                       synth_->filterRelease(), 0.0, 10000.0, 0.05);
}

void SynthSetting::updateParameterVisibility()
{
    const auto wavetype = (AUDIO::Waveform::Type)p_waveform_->baseValue();
    const auto filtertype = (AUDIO::MultiFilter::FilterType)p_filterType_->baseValue();
    const bool
            haspw = AUDIO::Waveform::supportsPulseWidth(wavetype),
            isfilter = filtertype != AUDIO::MultiFilter::T_BYPASS,
            hasorder = AUDIO::MultiFilter::supportsOrder(filtertype),
            hasreso = AUDIO::MultiFilter::supportsResonance(filtertype);

    p_pulseWidth_->setVisible(haspw);
    p_filterOrder_->setVisible(isfilter && hasorder);
    p_filterFreq_->setVisible(isfilter);
    p_filterReso_->setVisible(isfilter && hasreso);
    p_filterKeyFollow_->setVisible(isfilter);
    p_filterEnv_->setVisible(isfilter);
    p_filterEnvKeyFollow_->setVisible(isfilter);
    p_fattack_->setVisible(isfilter);
    p_fdecay_->setVisible(isfilter);
    p_fsustain_->setVisible(isfilter);
    p_frelease_->setVisible(isfilter);
}


bool SynthSetting::onParameterChanged(Parameter *p)
{
    if (p == p_numVoices_)
    {
        synth_->setNumberVoices(p_numVoices_->baseValue());
        voiceData_.resize(synth_->numberVoices());
        return true;
    }

    return false;
}

void SynthSetting::onParametersLoaded()
{
    synth_->setNumberVoices(p_numVoices_->baseValue());
    voiceData_.resize(synth_->numberVoices());
}

void SynthSetting::updateSynthParameters(Double time, uint thread)
{
    // notesPerOctave setting needs recalculation
    // so only set when changed
    const Double notesOct = p_notesPerOct_->value(time, thread);
    if (notesOct != synth_->notesPerOctave())
        synth_->setNotesPerOctave(notesOct);

    synth_->setVoicePolicy((AUDIO::Synth::VoicePolicy)
                           p_policy_->baseValue());
    synth_->setVolume(p_volume_->value(time, thread));
    synth_->setBaseFrequency(p_baseFreq_->value(time, thread));
    synth_->setCombinedUnison(p_combinedUnison_->value(time, thread));
    synth_->setUnisonVoices(p_numUnison_->value(time,thread));
    synth_->setUnisonNoteStep(p_unisonNoteStep_->value(time, thread));
    synth_->setUnisonDetune(p_unisonDetune_->value(time, thread));
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
}


void SynthSetting::feedSynthOnce(Double time, uint thread)
{
    Double gate = gate_->input(p_gate_->value(time, thread));

    if (gate > 0)
    {
        updateSynthParameters(time, thread);

        const int note = p_note_->value(time, thread);

        auto voice = synth_->noteOn(note, gate);
        if (voice)
        {
            // attach VoiceData
            auto data = &voiceData_[voice->index()];
            data->timeStarted = time;
            data->thread = thread;
            voice->setUserData(data);
        }
    }
}

void SynthSetting::feedSynth(SamplePos samplePos, uint thread, uint bufferSize)
{
    for (uint i=0; i<bufferSize; ++i)
    {
        const Double
                time = Double(samplePos + i) * o_->sampleRateInv(),
                gate = gate_->input(p_gate_->value(time, thread));

        if (gate > 0)
        {
            updateSynthParameters(time, thread);

            const int note = p_note_->value(time, thread);

            auto voice = synth_->noteOn(note, gate, i);
            while (voice)
            {
                // attach VoiceData
                auto data = &voiceData_[voice->index()];
                data->timeStarted = time;
                data->thread = thread;
                voice->setUserData(data);

                voice = voice->nextUnisonVoice();
            }
        }
    }
}

} // namespace MO
