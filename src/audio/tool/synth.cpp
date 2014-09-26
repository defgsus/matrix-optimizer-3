/** @file synth.cpp

    @brief Polyphonic synthesizer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 25.09.2014</p>
*/

#include "synth.h"
#include "io/log.h"

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
          velo		(0.0),
          fenvAmt   (0),
          filterFreq(0),
          lifetime	(0),
          waveform	(Waveform::T_SINE),
          filter    (true)

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
        fenvAmt,
        filterFreq;

    uint lifetime;

    EnvelopeGenerator<Double> env, fenv;

    Waveform::Type waveform;

    MultiFilter filter;


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
Double SynthVoice::attack() const { return p_->env.attack(); }
Double SynthVoice::decay() const { return p_->env.decay(); }
Double SynthVoice::sustain() const { return p_->env.sustain(); }
Double SynthVoice::release() const { return p_->env.release(); }
Double SynthVoice::filterAttack() const { return p_->fenv.attack(); }
Double SynthVoice::filterDecay() const { return p_->fenv.decay(); }
Double SynthVoice::filterSustain() const { return p_->fenv.sustain(); }
Double SynthVoice::filterRelease() const { return p_->fenv.release(); }
Double SynthVoice::filterEnvelopeAmount() const { return p_->fenvAmt; }
Waveform::Type SynthVoice::waveform() const { return p_->waveform; }
const EnvelopeGenerator<Double>& SynthVoice::envelope() const { return p_->env; }
const MultiFilter& SynthVoice::filter() const { return p_->filter; }

// ------------------------------------ synth private ------------------------------------

class Synth::Private
{
public:
    Private(Synth * s)
        : synth         (s),
          voicePolicy   (Synth::VP_OLDEST),
          sampleRate    (44100),
          filterOrder   (1),
          volume        (1.0),
          attack        (0.05),
          decay         (1.0),
          sustain       (0.0),
          release       (1.0),
          pulseWidth    (0.5),
          filterFreq    (1000.0),
          filterReso    (0.0),
          filterKeyFollow(0.0),
          filterEnvAmt  (0.0),
          filterAttack  (0.05),
          filterDecay   (1.0),
          filterSustain (0.0),
          filterRelease (1.0),
          waveform      (Waveform::T_SINE),
          filterType    (MultiFilter::T_BYPASS)
    {
        setNumVoices(4);
    }

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

    uint sampleRate, filterOrder;
    Double volume, attack, decay, sustain, release, pulseWidth,
            filterFreq, filterReso, filterKeyFollow,
            filterEnvAmt, filterAttack, filterDecay, filterSustain, filterRelease;

    Waveform::Type waveform;
    MultiFilter::FilterType filterType;

    NoteFreq<Double> noteFreq;
};


SynthVoice * Synth::Private::noteOn(uint startSample, Double freq, int note, Float velocity)
{
    if (voices.empty())
        return 0;

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

    //MO_DEBUG("start voice " << freq << "hz " << velocity);

    v->p_->freq = freq;
    // freq as coefficient
    v->p_->freq_c = v->p_->freq / sampleRate;
    v->p_->phase = 0.0;
    v->p_->pw = pulseWidth;
    v->p_->velo = velocity;
    v->p_->startSample = startSample;
    v->p_->env.setSampleRate(sampleRate);
    v->p_->env.setAttack(attack);
    v->p_->env.setDecay(decay);
    v->p_->env.setSustain(sustain);
    v->p_->env.setRelease(release);
    v->p_->fenv.setSampleRate(sampleRate);
    v->p_->fenv.setAttack(filterAttack);
    v->p_->fenv.setDecay(filterDecay);
    v->p_->fenv.setSustain(filterSustain);
    v->p_->fenv.setRelease(filterRelease);
    if (startSample == 0)
    {
        v->p_->env.trigger();
        v->p_->fenv.trigger();
    }
    v->p_->waveform = waveform;
    v->p_->lifetime = 0;
    v->p_->note = note;
    v->p_->active = startSample == 0;
    v->p_->filterFreq = filterFreq + filterKeyFollow * freq;
    v->p_->filter.setType(filterType);
    v->p_->filter.setOrder(filterOrder);
    v->p_->filter.setFrequency(v->p_->filterFreq);
    v->p_->filter.setResonance(filterReso);
    v->p_->filter.reset();
    v->p_->filter.updateCoefficients();
    v->p_->fenvAmt = filterEnvAmt;

    return v;
}

void Synth::Private::noteOff(uint /*stopSample*/, int note)
{
    // XXX implement stopSample cueing

    for (auto i : voices)
    if (i->p_->active && i->p_->note == note)
    {
        if (i->p_->env.release() > 0)
        {
            i->p_->env.setState(ENV_RELEASE);
            i->p_->fenv.setState(ENV_RELEASE);
        }
        else
        {
            i->p_->env.stop();
            i->p_->active = false;
        }
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
            if (v->startSample > 0 && v->startSample == sample)
            {
                v->env.trigger();
                v->active = true;
                v->startSample = 0;
            }

            if (!v->active)
                continue;

            // count number of samples alive
            ++(v->lifetime);

            // advance phase counter
            v->phase += v->freq_c;

            // get sample
            F32 s = Waveform::waveform(v->phase, v->waveform, v->pw);

            // filter
            if (v->filter.type() != MultiFilter::T_BYPASS)
                v->filter.process(&s, &s, 1);

            // put into buffer
            *output += s * volume * v->velo * v->env.value();

            // process envelop
            v->env.next();
            // check for end of envelop
            if (!v->env.active())
            {
                v->active = false;
                continue;
            }

            // process filter envelop
            if (v->filter.type() != MultiFilter::T_BYPASS && v->fenvAmt != 0)
            {
                v->filter.setFrequency(v->filterFreq + v->fenvAmt * v->fenv.next());
                v->filter.updateCoefficients();
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
Waveform::Type Synth::waveform() const { return p_->waveform; }
uint Synth::filterOrder() const { return p_->filterOrder; }
Double Synth::filterFrequency() const { return p_->filterFreq; }
Double Synth::filterResonance() const { return p_->filterReso; }
Double Synth::filterKeyFollower() const { return p_->filterKeyFollow; }
MultiFilter::FilterType Synth::filterType() const { return p_->filterType; }
Double Synth::filterEnvelopeAmount() const { return p_->filterEnvAmt; }
Double Synth::filterAttack() const { return p_->filterAttack; }
Double Synth::filterDecay() const { return p_->filterDecay; }
Double Synth::filterSustain() const { return p_->filterSustain; }
Double Synth::filterRelease() const { return p_->filterRelease; }

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
void Synth::setWaveform(Waveform::Type t) { p_->waveform = t; }
void Synth::setFilterOrder(uint v) { p_->filterOrder = std::max(uint(1), v); }
void Synth::setFilterFrequency(Double v) { p_->filterFreq = v; }
void Synth::setFilterResonance(Double v) { p_->filterReso = v; }
void Synth::setFilterKeyFollower(Double v) { p_->filterKeyFollow = v; }
void Synth::setFilterType(MultiFilter::FilterType t) { p_->filterType = t; }
void Synth::setFilterEnvelopeAmount(Double v) { p_->filterEnvAmt = v; }
void Synth::setFilterAttack(Double v) { p_->filterAttack = v; }
void Synth::setFilterDecay(Double v) { p_->filterDecay = v; }
void Synth::setFilterSustain(Double v) { p_->filterSustain = v; }
void Synth::setFilterRelease(Double v) { p_->filterRelease = v; }

SynthVoice * Synth::noteOn(int note, Float velocity)
{
    return p_->noteOn(0, p_->noteFreq.frequency(note), note, velocity);
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
