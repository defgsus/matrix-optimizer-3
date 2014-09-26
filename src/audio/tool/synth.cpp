/** @file synth.cpp

    @brief Polyphonic synthesizer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 25.09.2014</p>
*/

#include "synth.h"

namespace MO {
namespace AUDIO {

// ------------------------------------ synthvoice private ------------------------------------

class SynthVoice::Private
{
    public:

    Private(Synth * synth)
        : synth     (synth),
          active	(false),
          note   	(0),
          startSample(0),
          freq		(0.0),
          freq_c	(0.0),
          phase		(0.0),
          pw        (0.5),
          velo		(0.f),
          env		(0.f),
          attack	(0.f),
          decay 	(0.f),
          sustain	(0.f),
          release	(0.f),
          envstate	(SynthVoice::ENV_ATTACK),
          waveform	(Waveform::T_SINE),
          lifetime	(0)
    { }

    Synth * synth;

    bool active;

    int note;
    uint startSample;

    Double
        freq,
        freq_c,
        phase,
        pw,
        velo,
        env,
        attack,
        decay,
        sustain,
        release;

    SynthVoice::EnvelopeState envstate;

    Waveform::Type waveform;

    uint lifetime;
};

// ------------------------------------ synthvoice ------------------------------------

SynthVoice::SynthVoice(Synth * synth)
    : p_    (new Private(synth))
{
}

SynthVoice::~SynthVoice()
{
    delete p_;
}

Synth * SynthVoice::synth() const { return p_->synth; }
bool SynthVoice::active() const { return p_->active; }
int SynthVoice::note() const { return p_->note; }
Double SynthVoice::freq() const { return p_->freq; }
Double SynthVoice::phase() const { return p_->phase; }
Double SynthVoice::pulseWidth() const { return p_->pw; }
Double SynthVoice::velocity() const { return p_->velo; }
Double SynthVoice::attack() const { return p_->attack; }
Double SynthVoice::decay() const { return p_->decay; }
Double SynthVoice::sustain() const { return p_->sustain; }
Double SynthVoice::release() const { return p_->release; }
Waveform::Type SynthVoice::waveform() const { return p_->waveform; }
SynthVoice::EnvelopeState SynthVoice::envelopeState() const { return p_->envstate; }


// ------------------------------------ synth private ------------------------------------

class Synth::Private
{
public:
    Private(Synth * s)
        : synth         (s),
          voicePolicy   (Synth::VP_OLDEST),
          sampleRate    (44100),
          volume        (1.0),
          attack        (0.05),
          decay         (1.0),
          sustain       (0.0),
          release       (0.0),
          pulseWidth    (0.5),
          waveform      (Waveform::T_SINE)
    { }

    ~Private()
    {
        deleteVoices();
    }

    void deleteVoices()
    {
        for (auto v : voices)
            delete v;
        voices.clear();
    }

    void setNumVoices(uint n)
    {
        deleteVoices();
        voices.resize(n);
        for (auto & v : voices)
            v = new SynthVoice(synth);
    }

    SynthVoice * noteOn(uint startSample, Double freq, int note, Float velocity);
    void noteOff(uint stopSample, int note);
    void process(F32 * output, uint bufferLength);

    Synth * synth;

    std::vector<SynthVoice*> voices;

    Synth::VoicePolicy voicePolicy;

    uint sampleRate;
    Double volume, attack, decay, sustain, release, pulseWidth;
    Waveform::Type waveform;

    NoteFreq<Double> noteFreq;
};


SynthVoice * Synth::Private::noteOn(uint startSample, Double freq, int note, Float velocity)
{
    // find free (non-active) voice
    auto i = voices.begin();
    for (; i!=voices.end(); ++i)
        if (!(*i)->p_->active) break;

    // none free?
    if (i == voices.end())
    {
        if (voicePolicy == Synth::VP_FORGET) return 0;

        i = voices.begin();

        // policy doesn't matter for monophonic synth
        if (voices.size()>1)
        {
            // decide which to overwrite

            if (voicePolicy == Synth::VP_LOWEST)
            {
                auto j = i;
                Double f = (*i)->p_->freq;
                ++j;
                for (; j != voices.end(); ++j)
                if ((*j)->p_->freq < f)
                {
                    f = (*j)->p_->freq;
                    i = j;
                }
            }
            else
            if (voicePolicy == Synth::VP_HIGHEST)
            {
                auto j = i;
                Double f = (*i)->p_->freq;
                ++j;
                for (; j != voices.end(); ++j)
                if ((*j)->p_->freq > f)
                {
                    f = (*j)->p_->freq;
                    i = j;
                }
            }
            else
            //if (voicePolicy == Synth::VP_OLDEST)
            {
                auto j = i;
                uint l = (*i)->p_->lifetime;
                ++j;
                for (; j != voices.end(); ++j)
                if ((*j)->p_->lifetime > l)
                {
                    l = (*j)->p_->lifetime;
                    i = j;
                }
            }
        }
    }

    // (re-)init voice

    SynthVoice * v = *i;

    v->p_->freq = freq;
    // freq as coefficient
    v->p_->freq_c = v->p_->freq / sampleRate;
    v->p_->phase = 0.0;
    v->p_->pw = pulseWidth;
    v->p_->velo = velocity;
    v->p_->env = 0.0;
    v->p_->envstate = startSample > 0 ? SynthVoice::ENV_CUED : SynthVoice::ENV_ATTACK;
    v->p_->startSample = startSample;
    // envelope times as coefficients
    v->p_->attack  = 8.0 / std::max(8.0, attack * sampleRate);
    v->p_->decay   = 8.0 / std::max(8.0, decay * sampleRate);
    v->p_->release = 8.0 / std::max(8.0, release * sampleRate);
    v->p_->sustain = sustain;
    v->p_->waveform = waveform;
    v->p_->lifetime = 0;
    v->p_->note = note;
    v->p_->active = true;

    return v;
}

void Synth::Private::noteOff(uint /*stopSample*/, int note)
{
    // XXX implement stopSample cueing

    for (auto i : voices)
    if (i->p_->active && i->p_->note == note)
    {
        if (i->p_->release > 0)
            i->p_->envstate = SynthVoice::ENV_RELEASE;
        else
            i->p_->active = false;
    }
}


void Synth::Private::process(F32 *output, uint bufferLength)
{
    memset(output, 0, sizeof(F32) * bufferLength);

    // for each sample
    for (uint sample = 0; sample < bufferLength; ++sample, ++output)
    {
        // for each voice
        for (auto i : voices)
        {
            SynthVoice::Private * v = i->p_;

            // start cued voice
            if (v->envstate == SynthVoice::ENV_CUED
                && v->startSample == sample)
            {
                v->envstate = SynthVoice::ENV_ATTACK;
                v->active = true;
            }

            if (!v->active)
                continue;

            // count number of samples alive
            ++(v->lifetime);

            // advance phase counter
            v->phase += v->freq_c;

            // get sample
            F32 s = Waveform::waveform(v->phase, v->waveform, v->pw);

            // put into buffer
            *output += s * volume * v->velo * v->env;

            // process envelope
            switch (v->envstate)
            {
                case SynthVoice::ENV_ATTACK:
                    v->env += v->attack * (1.0 - v->env);
                    if (v->env >= 0.999)
                        v->envstate = SynthVoice::ENV_RELEASE;
                break;

                case SynthVoice::ENV_DECAY:
                    v->env += v->decay * (v->sustain - v->env);
                    if (v->env <= v->sustain*1.001)
                        v->envstate = SynthVoice::ENV_RELEASE;
                break;

                case SynthVoice::ENV_RELEASE:
                    v->env -= v->release * v->env;
                    if (v->env <= 0.0001)
                    {
                        v->active = false;
                        v->env = 0.0;
                    }
                break;

                case SynthVoice::ENV_SUSTAIN:
                case SynthVoice::ENV_CUED:
                break;
            }
        }
    }
}


// ------------------------------------ synth ------------------------------------

Synth::Synth()
    : p_    (new Private(this))
{

}

Synth::~Synth()
{
    delete p_;
}

uint Synth::sampleRate() const { return p_->sampleRate; }
uint Synth::numberVoices() const { return p_->voices.size(); }
Double Synth::volume() const { return p_->volume; }
Double Synth::attack() const { return p_->attack; }
Double Synth::decay() const { return p_->decay; }
Double Synth::sustain() const { return p_->sustain; }
Double Synth::release() const { return p_->release; }
Double Synth::pulseWidth() const { return p_->pulseWidth; }
const NoteFreq<Double>& Synth::noteFreq() const { return p_->noteFreq; }
Double Synth::notesPerOctave() const { return p_->noteFreq.notesPerOctave(); }
Double Synth::baseFrequency() const { return p_->noteFreq.baseFrequency(); }

void Synth::setNumberVoices(uint v) { p_->setNumVoices(v); }
void Synth::setSampleRate(uint v) { p_->sampleRate = v; }
void Synth::setVolume(Double v) { p_->volume = v; }
void Synth::setAttack(Double v) { p_->attack = v; }
void Synth::setDecay(Double v) { p_->decay = v; }
void Synth::setSustain(Double v) { p_->sustain = v; }
void Synth::setRelease(Double v) { p_->release = v; }
void Synth::setPulseWidth(Double v) { p_->pulseWidth = Waveform::limitPulseWidth(v); }
void Synth::setNoteFreq(const NoteFreq<Double>& n) { p_->noteFreq = n; }
void Synth::setNotesPerOctave(Double v) { p_->noteFreq.setNotesPerOctave(v); }
void Synth::setBaseFrequency(Double v) { p_->noteFreq.setBaseFrequency(v); }

SynthVoice * Synth::noteOn(int note, Float velocity)
{
    return p_->noteOn(0, 0.0, note, velocity);
}

void Synth::noteOff(int note)
{
    p_->noteOff(0, note);
}

void Synth::process(F32 *output, uint bufferLength)
{
    p_->process(output, bufferLength);
}



} // namespace AUDIO
} // namespace MO
