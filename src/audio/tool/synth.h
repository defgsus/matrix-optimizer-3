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

    /** Returns the next unisono voice, or NULL */
    SynthVoice * nextUnisonVoice() const;

    /** Returns the data previously set with setUserData(),
        or NULL if the voice was created by Synth::notOn() */
    void * userData() const;

    // ----------- setter --------------

    /** Freely chooseable user data */
    void setUserData(void * data);

private:

    class Private;
    Private * p_;
};



/** A stand-alone polyphonic synthesizer */
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
    /** Sets the polyphony of the synthesizer.
        Currently, this immidiately stops and destroys all voices,
        so all voices you got from noteOn() will be invalid! */
    void setNumberVoices(uint num);

    // below values are used per voice on noteOn()

    /** Master volume of the synthesizer */
    void setVolume(Double volume);
    /** Number of actual voices triggered for each noteOn() */
    void setUnisonVoices(uint num);
    /** Changes the unisono mode.
        If @p combined is true, no additional voices will be created.
        Instead each voice will generate unisonVoices() number of sounds
        which all go through the same filter and envelope.
        If @p combined is false, each unisono voice will be an
        actual SynthVoice. */
    void setCombinedUnison(bool combined);
    /** Sets the random detuning of each additional unisono voice in cents
        (100 cents == one note). */
    void setUnisonDetune(Double cent);
    /** Sets the note that is added for each additional unisono voice. */
    void setUnisonNoteStep(int step);

    /** Attack time in seconds */
    void setAttack(Double attack);
    /** Decay time in seconds */
    void setDecay(Double decay);
    /** Sustain level */
    void setSustain(Double sustain);
    /** Release time in seconds. */
    void setRelease(Double release);
    /** Pulsewidth for oscillator types that support it, 0 < @p pw < 1.
        See AUDIO::Waveform::supportsPulseWidth() */
    void setPulseWidth(Double pw);

    /** Replaces the note-to-frequency setting.s */
    void setNoteFreq(const NoteFreq<Double>& n);
    /** Sets the number of notes per octave. */
    void setNotesPerOctave(Double notes);
    /** Sets the frequency in Hertz of the lowest C note */
    void setBaseFrequency(Double f);

    /** Sets the waveform of the oscillator */
    void setWaveform(Waveform::Type t);

    void setFilterType(MultiFilter::FilterType t);
    void setFilterOrder(uint order);
    void setFilterFrequency(Double freq);
    void setFilterResonance(Double res);
    /** Sets the key-follower for the filter.
        The note frequency is multiplied by @p amp and added to the
        set filter frequency.
        E.g. if filterFrequency() == 0 and filterKeyFollower() == 1,
        the filter frequency will be exactly the note frequency. */
    void setFilterKeyFollower(Double amt);
    void setFilterEnvelopeAmount(Double env);
    void setFilterEnvelopeKeyFollower(Double amt);
    void setFilterAttack(Double attack);
    void setFilterDecay(Double decay);
    void setFilterSustain(Double sustain);
    void setFilterRelease(Double release);

    // ---------- audio -------------------

    /** Starts the next free voice.
        Returns the triggered voice, or NULL if no free voice was found and
        the VoicePolicy doesn't allow for stopping a voice.
        For true unisono voices (when unisonVoices() > 1 and combinedUnison() == true),
        the additional voices are in SynthVoice::nextUnisonVoice().
        If @p startSample > 0, the start of the voice will be delayed for the
        given number of samples. The returned voice will be valid but not active yet.
        Note that startSample must be smaller than the bufferLength parameter
        in process() to start the voice. */
    SynthVoice * noteOn(int note, Float velocity, uint startSample = 0);

    /** Stops all active voices of the given note.
        Depending on their sustain level, they will immidiately stop
        (sustain==0) or enter into RELEASE evelope state. */
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


