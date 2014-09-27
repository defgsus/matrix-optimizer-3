/** @file synth.h

    @brief Polyphonic synthesizer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 25.09.2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_SYNTH_H
#define MOSRC_AUDIO_TOOL_SYNTH_H

#include "waveform.h"
#include "notefreq.h"
#include "multifilter.h"
#include "envelopegenerator.h"

namespace MO {
namespace AUDIO {

class Synth;

/** One synthesizer voice */
class SynthVoice
{
    friend class Synth;
public:

    SynthVoice(Synth *);
    ~SynthVoice();

    // ------------ getter -------------

    Synth * synth() const;
    bool active() const;
    int note() const;
    Double freq() const;
    Double phase() const;
    Double pulseWidth() const;
    Double velocity() const;
    Double attack() const;
    Double decay() const;
    Double sustain() const;
    Double release() const;
    Double filterEnvelopeAmount() const;
    Double filterAttack() const;
    Double filterDecay() const;
    Double filterSustain() const;
    Double filterRelease() const;

    Waveform::Type waveform() const;

    const EnvelopeGenerator<Double>& envelope() const;
    const EnvelopeGenerator<Double>& filterEnvelope() const;

    const MultiFilter& filter() const;

    /** Returns the next unison voice, or NULL */
    SynthVoice * nextUnisonVoice() const;

private:

    class Private;
    Private * p_;
};


class Synth
{
public:

    /** Voice reuse policy when reached max polyphony */
    enum VoicePolicy
    {
        /** Forget new voice */
        VP_FORGET,
        /** Stop oldest voice */
        VP_OLDEST,
        /** Stop lowest voice */
        VP_LOWEST,
        /** Stop highest voice */
        VP_HIGHEST
    };

    Synth();
    ~Synth();

    // ------------ getter ----------------

    uint sampleRate() const;
    uint numberVoices() const;
    Double volume() const;
    bool combinedUnison() const;
    uint unisonVoices() const;
    Double unisonDetune() const;
    int unisonNoteStep() const;

    Double attack() const;
    Double decay() const;
    Double sustain() const;
    Double release() const;
    Double pulseWidth() const;

    const NoteFreq<Double>& noteFreq() const;
    Double notesPerOctave() const;
    Double baseFrequency() const;

    Waveform::Type waveform() const;

    MultiFilter::FilterType filterType() const;
    uint filterOrder() const;
    Double filterFrequency() const;
    Double filterResonance() const;
    Double filterKeyFollower() const;
    Double filterEnvelopeAmount() const;
    Double filterEnvelopeKeyFollower() const;
    Double filterAttack() const;
    Double filterDecay() const;
    Double filterSustain() const;
    Double filterRelease() const;

    // ----------- setter -----------------

    void setSampleRate(uint sr);
    void setNumberVoices(uint num);
    void setVolume(Double volume);
    void setCombinedUnison(bool combined);
    void setUnisonVoices(uint num);
    void setUnisonDetune(Double cent);
    void setUnisonNoteStep(int step);

    // below values are used per voice on noteOn()

    void setAttack(Double attack);
    void setDecay(Double decay);
    void setSustain(Double sustain);
    void setRelease(Double release);
    void setPulseWidth(Double pw);

    void setNoteFreq(const NoteFreq<Double>& n);
    void setNotesPerOctave(Double notes);
    void setBaseFrequency(Double f);

    void setWaveform(Waveform::Type t);

    void setFilterType(MultiFilter::FilterType t);
    void setFilterOrder(uint order);
    void setFilterFrequency(Double freq);
    void setFilterResonance(Double res);
    void setFilterKeyFollower(Double amt);
    void setFilterEnvelopeAmount(Double env);
    void setFilterEnvelopeKeyFollower(Double amt);
    void setFilterAttack(Double attack);
    void setFilterDecay(Double decay);
    void setFilterSustain(Double sustain);
    void setFilterRelease(Double release);

    // ---------- audio -------------------

    /** Starts the next free voice.
        Returns the triggered voice or NULL. */
    SynthVoice * noteOn(int note, Float velocity);

    /** Puts all active voices of the given note into RELEASE state */
    void noteOff(int note);

    /** Generates @p bufferLength samples of synthesizer music */
    void process(F32 * output, uint bufferLength);

private:

    class Private;
    Private * p_;
};


} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_SYNTH_H


