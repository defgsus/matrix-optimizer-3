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

namespace MO {
namespace AUDIO {

class Synth;

/** One synthesizer voice */
class SynthVoice
{
    friend class Synth;
public:

    enum EnvelopeState
    {
        ENV_CUED,
        ENV_ATTACK,
        ENV_DECAY,
        ENV_SUSTAIN,
        ENV_RELEASE
    };

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
    Waveform::Type waveform() const;
    EnvelopeState envelopeState() const;

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
    Double attack() const;
    Double decay() const;
    Double sustain() const;
    Double release() const;
    Double pulseWidth() const;

    const NoteFreq<Double>& noteFreq() const;
    Double notesPerOctave() const;
    Double baseFrequency() const;

    Waveform::Type waveform() const;

    // ----------- setter -----------------

    void setSampleRate(uint sr);
    void setNumberVoices(uint num);
    void setVolume(Double volume);
    void setAttack(Double attack);
    void setDecay(Double decay);
    void setSustain(Double sustain);
    void setRelease(Double release);
    void setPulseWidth(Double pw);

    void setNoteFreq(const NoteFreq<Double>& n);
    void setNotesPerOctave(Double notes);
    void setBaseFrequency(Double f);

    void setWaveform(Waveform::Type t);

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


