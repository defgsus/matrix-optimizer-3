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
          freq		(0.0),
          freq_c	(0.0),
          phase		(0.0),
          velo		(0.f),
          env		(0.f),
          attack	(0.f),
          decay 	(0.f),
          sustain	(0.f),
          release	(0.f),
          envstate	(ENV_ATTACK),
          waveform	(Waveform::T_SINE),
          time		(0.0),
          lifetime	(0)
    { }

    Synth * synth;

    bool active;

    int note;
    Double
        freq,
        freq_c,
        phase;
    Float
        velo,
        env,
        attack,
        decay,
        sustain,
        release;

    EnvelopeState envstate;

    Waveform::Type waveform;

    Double time;

    size_t lifetime;
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
          attack        (0.05),
          decay         (1.0),
          sustain       (0.0),
          release       (0.0),
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

    SynthVoice * noteOn(Double time, Double freq, int note, Float velocity);

    Synth * synth;

    std::vector<SynthVoice*> voices;

    Synth::VoicePolicy voicePolicy;

    int sampleRate;
    Double attack, decay, sustain, release;
    Waveform::Type waveform;
};

SynthVoice * Synth::Private::noteOn(Double time, Double freq, int note, Float velocity)
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
                auto l = (*i)->p_->lifetime;
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
    v->p_->velo = velocity;
    v->p_->env = 0.0;
    v->p_->envstate = SynthVoice::ENV_ATTACK;
    // envelope times as coefficients
    v->p_->attack  = 8.0 / std::max(8.0, attack * sampleRate);
    v->p_->decay   = 8.0 / std::max(8.0, decay * sampleRate);
    v->p_->release = 8.0 / std::max(8.0, release * sampleRate);
    v->p_->sustain = sustain;
    v->p_->waveform = waveform;
    v->p_->time = time;
    v->p_->lifetime = 0;
    v->p_->note = note;
    v->p_->active = true;

    return v;
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


SynthVoice * Synth::noteOn(Double time, int note, Float velocity)
{
    return p_->noteOn(time, 0.0, note, velocity);
}



} // namespace AUDIO
} // namespace MO
