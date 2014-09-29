/** @file synth.cpp

    @brief Polyphonic synthesizer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 25.09.2014</p>
*/

#include <QObject> // for tr()

#include "synth.h"
#include "io/log.h"
#include "math/random.h"

#if (0)
#   define MO_DEBUG_SYNTH(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_SYNTH(unused__)
#endif


namespace MO {
namespace AUDIO {

// ------------------------------------ synthvoice private ------------------------------------

class SynthVoice::Private
{
    public:

    Private(Synth * synth)
        : synth     (synth),
          active	(false),
          cued      (false),
          note   	(0),
          index     (0),
          startSample(0),
          freq		(0.0),
          pw        (0.5),
          velo		(0.0),
          fenvAmt   (0),
          filterFreq(0),
          lifetime	(0),
          waveform	(Waveform::T_SINE),
          filter    (true),
          nextUnison(0),
          data      (0)

    { }

    Double curLevel() const { return velo * env.value(); }

    Synth * synth;

    bool active, cued;

    int note;
    uint index, startSample;

    Double
        freq,
        pw,
        velo,
        fenvAmt,
        filterFreq;

    std::vector<Double>
        freq_c,
        phase;

    uint lifetime;

    EnvelopeGenerator<Double> env, fenv;

    Waveform::Type waveform;

    MultiFilter filter;

    SynthVoice * nextUnison;

    void * data;
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
uint SynthVoice::index() const { return p_->index; }
bool SynthVoice::active() const { return p_->active; }
bool SynthVoice::cued() const { return p_->cued; }
int SynthVoice::note() const { return p_->note; }
uint SynthVoice::startSample() const { return p_->startSample; }
Double SynthVoice::freq() const { return p_->freq; }
Double SynthVoice::phase() const { return p_->phase[0]; }
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
SynthVoice * SynthVoice::nextUnisonVoice() const { return p_->nextUnison; }
void * SynthVoice::userData() const { return p_->data; }

void SynthVoice::setUserData(void *data) { p_->data = data; }







// ------------------------------------ synth private ------------------------------------

class Synth::Private
{
public:
    Private(Synth * s)
        : synth         (s),
          voicePolicy   (Synth::VP_QUITEST),
          sampleRate    (44100),
          filterOrder   (1),
          unisonVoices  (1),
          unisonNoteStep(0),
          combinedUnison(true),
          volume        (1.0),
          unisonDetune  (20),
          attack        (0.05),
          decay         (1.0),
          sustain       (0.0),
          release       (1.0),
          pulseWidth    (0.5),
          filterFreq    (1000.0),
          filterReso    (0.0),
          filterKeyFollow(0.0),
          filterEnvAmt  (0.0),
          filterEnvKeyFollow(0.0),
          filterAttack  (0.05),
          filterDecay   (1.0),
          filterSustain (0.0),
          filterRelease (1.0),
          waveform      (Waveform::T_SINE),
          filterType    (MultiFilter::T_BYPASS),
          cbStart_      (0),
          cbEnd_        (0)
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
        {
            if (v->p_->active && synth->p_->cbEnd_)
                synth->p_->cbEnd_(v);
            delete v;
        }
        voices.clear();
    }

    void setNumVoices(uint n)
    {
        deleteVoices();
        voices.resize(n);
        for (uint i=0; i<n; ++i)
        {
            voices[i] = new SynthVoice(synth);
            voices[i]->p_->index = i;
        }
    }

    SynthVoice * noteOn(uint startSample, Double freq, int note, Float velocity,
                        uint numCombinedUnison, void *userData);
    void noteOff(uint stopSample, int note);
    /** Mono output */
    void process(F32 * output, uint bufferLength);
    /** Multichannel output */
    void process(F32 ** output, uint bufferLength);

    Synth * synth;

    std::vector<SynthVoice*> voices;

    Synth::VoicePolicy voicePolicy;

    uint sampleRate, filterOrder, unisonVoices, unisonNoteStep;
    bool combinedUnison;
    Double  volume, unisonDetune,
            attack, decay, sustain, release, pulseWidth,
            filterFreq, filterReso, filterKeyFollow,
            filterEnvAmt, filterEnvKeyFollow,
            filterAttack, filterDecay, filterSustain, filterRelease;

    Waveform::Type waveform;
    MultiFilter::FilterType filterType;

    NoteFreq<Double> noteFreq;

    std::function<void(SynthVoice*)>
        cbStart_, cbEnd_;
};


SynthVoice * Synth::Private::noteOn(uint startSample, Double freq, int note, Float velocity,
                                    uint numCombinedUnison, void * userData)
{
    if (voices.empty())
        return 0;

    // find free (non-active & non-cued) voice
    auto i = voices.begin();
    for (; i!=voices.end(); ++i)
        if (!(*i)->p_->active && !(*i)->p_->cued) break;

    // none free?
    if (i == voices.end())
    {
        if (voicePolicy == Synth::VP_FORGET) return 0;

        MO_DEBUG_SYNTH("Synth::noteOn() looking for voice to reuse");

        i = voices.begin();

        // policy doesn't matter for monophonic synth
        if (voices.size()>1)
        {
            // decide which to overwrite

            // this macro compares the value__s of each voice
            // and returns the voice to reuse in 'i'
            // it also checks for the envelope state and
            // rather reuses voices that are at a later state
            #define MO__FIND_VOICE_ENV(value__, operator__)                 \
            {                                                               \
                auto j = i;                                                 \
                auto val1 = (*i)->p_->value__;                              \
                auto val2 = val1;                                           \
                ++j;                                                        \
                for (; j != voices.end(); ++j)                              \
                {                                                           \
                    auto val = (*j)->p_->value__;                           \
                    if (val operator__ val1                                 \
                        && (*j)->p_->env.state() > (*i)->p_->env.state())   \
                    {                                                       \
                        val1 = val;                                         \
                        i = j;                                              \
                    }                                                       \
                    else if (val operator__ val2)                           \
                    {                                                       \
                        val2 = val;                                         \
                        i = j;                                              \
                    }                                                       \
                }                                                           \
            }

            // same as above but does not care for envelope states
            #define MO__FIND_VOICE_NO_ENV(value__, operator__)              \
            {                                                               \
                auto j = i;                                                 \
                auto val1 = (*i)->p_->value__;                              \
                ++j;                                                        \
                for (; j != voices.end(); ++j)                              \
                {                                                           \
                    auto val = (*j)->p_->value__;                           \
                    f (val operator__ val1)                                 \
                    {                                                       \
                        val1 = val;                                         \
                        i = j;                                              \
                    }                                                       \
                }                                                           \
            }

            // XXX place for branching with a yet missing option in Synth
            #define MO__FIND_VOICE(value__, operator__) \
                    MO__FIND_VOICE_ENV(value__, operator__)

            switch (voicePolicy)
            {
                case Synth::VP_LOWEST:
                    MO__FIND_VOICE(freq, <);
                break;

                case Synth::VP_HIGHEST:
                    MO__FIND_VOICE(freq, >);
                break;

                case Synth::VP_OLDEST:
                    MO__FIND_VOICE(lifetime, >);
                break;

                case Synth::VP_NEWEST:
                    MO__FIND_VOICE(lifetime, <);
                break;

                case Synth::VP_QUITEST:
                    MO__FIND_VOICE(curLevel(), <);
                break;

                case Synth::VP_LOUDEST:
                    MO__FIND_VOICE(curLevel(), >);
                break;

                case Synth::VP_FORGET: break;
            }

            #undef MO__FIND_VOICE
            #undef MO__FIND_VOICE_ENV
            #undef MO__FIND_VOICE_NO_ENV
        }

        MO_DEBUG_SYNTH("Synth::noteOn(): reusing voice " << (*i)->p_->index);
    }

    // (re-)init voice

    SynthVoice::Private * v = (*i)->p_;

    MO_DEBUG_SYNTH("init voice " << v->index << " f=" << freq << "hz " << " v=" << velocity);

    v->lifetime = 0;
    v->active = false;
    v->cued = true;
    v->note = note;

    v->freq = freq;

    // freq as coefficient
    const Double freqc = v->freq / sampleRate;
    v->freq_c.resize(numCombinedUnison);
    v->phase.resize(numCombinedUnison);
    for (uint i=0; i<numCombinedUnison; ++i)
    {
        v->freq_c[i] = freqc;
        v->phase[i] = 0.0;
    }
    v->pw = pulseWidth;
    v->velo = velocity;
    v->startSample = startSample;
    v->env.setSampleRate(sampleRate);
    v->env.setAttack(attack);
    v->env.setDecay(decay);
    v->env.setSustain(sustain);
    v->env.setRelease(release);
    v->fenv.setSampleRate(sampleRate);
    v->fenv.setAttack(filterAttack);
    v->fenv.setDecay(filterDecay);
    v->fenv.setSustain(filterSustain);
    v->fenv.setRelease(filterRelease);
    v->waveform = waveform;
    v->filterFreq = filterFreq + filterKeyFollow * freq;
    v->filter.setType(filterType);
    v->filter.setOrder(filterOrder);
    v->filter.setFrequency(v->filterFreq);
    v->filter.setResonance(filterReso);
    v->filter.reset();
    v->filter.updateCoefficients();
    v->fenvAmt = filterEnvAmt + filterEnvKeyFollow * freq;
    v->data = userData;
    v->nextUnison = 0;

    return *i;
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
            // XXX implement noteEnd callback

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
        for (SynthVoice * i : voices)
        {
            SynthVoice::Private * v = i->p_;

            // start cued voice
            if (v->cued && v->startSample == sample)
            {
                v->env.trigger();
                if (v->fenvAmt)
                    v->fenv.trigger();
                v->active = true;
                v->cued = false;
                if (cbStart_)
                    cbStart_(i);
            }

            if (!v->active)
                continue;

            // count number of samples alive
            ++(v->lifetime);

            F32 s = 0.0;
            for (uint j = 0; j<v->phase.size(); ++j)
            {
                // advance phase counter
                v->phase[j] += v->freq_c[j];
                // get sample
                s += Waveform::waveform(v->phase[j], v->waveform, v->pw);
            }

            // filter
            if (v->filter.type() != MultiFilter::T_BYPASS)
                v->filter.process(&s, &s, 1);

            // put into buffer
            *output += s * volume * v->velo * v->env.value();

            // process envelope
            v->env.next();
            // check for end of envelope
            if (!v->env.active())
            {
                v->active = false;
                if (cbEnd_)
                    cbEnd_(i);
                continue;
            }

            // process filter envelope
            if (v->filter.type() != MultiFilter::T_BYPASS && v->fenvAmt != 0)
            {
                v->filter.setFrequency(v->filterFreq + v->fenvAmt * v->fenv.next());
                v->filter.updateCoefficients();
            }
        }
    }
}

void Synth::Private::process(F32 ** outputs, uint bufferLength)
{
    // for each voice
    for (uint voicecount = 0; voicecount < voices.size(); ++voicecount)
    {
        F32 * output = outputs[voicecount];
        SynthVoice::Private * v = voices[voicecount]->p_;

        // if voice is inactive (and won't become active)
        if (!v->active && !v->cued)
        {
            //MO_DEBUG_SYNTH("unused " << v->index);

            // clear output buffer
            memset(output, 0, sizeof(F32) * bufferLength);

            continue;
        }

        // where to start rendering
        uint start = 0;

        // start voice?
        if (v->cued)
        {
            if (v->startSample < bufferLength)
            {
                MO_DEBUG_SYNTH("start cued voice " << v->index << " s=" << v->startSample);

                // start at specified start-sample
                start = v->startSample;
                v->env.trigger();
                if (v->fenvAmt)
                    v->fenv.trigger();
                v->active = true;
                v->cued = false;
                // send callback
                if (cbStart_)
                    cbStart_(voices[voicecount]);

                // clear first part of buffer
                memset(output, 0, sizeof(F32) * start);
                output += start;
            }
            else
            {
                // if startsample is out of range
                // don't check again
                v->cued = false;
                continue;
            }
        }

        //MO_DEBUG("process " << v->index << " " << start << "-" << bufferLength);

        // for each sample
        for (uint sample = start; sample < bufferLength; ++sample, ++output)
        {
            // get oscillator sample
            F32 s = 0.0;
            for (uint j = 0; j<v->phase.size(); ++j)
            {
                // advance phase counter
                v->phase[j] += v->freq_c[j];
                // get sample
                s += Waveform::waveform(v->phase[j], v->waveform, v->pw);
            }

            // envelope before filter
            s *= v->env.value();

            // filter
            // TODO: For non-enveloped filters,
            //       we could process the whole buffer at once
            if (v->filter.type() != MultiFilter::T_BYPASS)
                v->filter.process(&s, &s, 1);

            // put into buffer
            *output = s * volume * v->velo;

            // process envelope
            v->env.next();
            // check for end of envelope
            if (!v->env.active())
            {
                MO_DEBUG_SYNTH("voice end " << v->index << " s=" << sample);

                v->active = false;
                // clear rest of buffer
                if (sample < bufferLength - 1)
                    memset(output+1, 0, sizeof(F32) * (bufferLength - sample - 1));
                // send callback
                if (cbEnd_)
                    cbEnd_(voices[voicecount]);
                break;
            }

            // process filter envelope
            if (v->filter.type() != MultiFilter::T_BYPASS && v->fenvAmt != 0)
            {
                v->filter.setFrequency(v->filterFreq + v->fenvAmt * v->fenv.next());
                v->filter.updateCoefficients();
            }
        }

        // count number of samples alive
        v->lifetime += bufferLength - start;
    }
}

// ------------------------------------ synth ------------------------------------

const QStringList Synth::voicePolicyIds =
{
    "fgt",
    "low", "high",
    "old", "new",
    "quite", "loud"
};

const QStringList Synth::voicePolicyNames =
{
    QObject::tr("forget"),
    QObject::tr("lowest"),
    QObject::tr("highest"),
    QObject::tr("oldest"),
    QObject::tr("newest"),
    QObject::tr("quitest"),
    QObject::tr("loudest")
};

const QStringList Synth::voicePolicyStatusTips =
{
    QObject::tr("Don't start a new voice when maximum polyphony is reached"),
    QObject::tr("Stop and reuse the lowest voice when maximum polyphony is reached"),
    QObject::tr("Stop and reuse the highest voice when maximum polyphony is reached"),
    QObject::tr("Stop and reuse the oldest voice when maximum polyphony is reached"),
    QObject::tr("Stop and reuse the newest voice when maximum polyphony is reached"),
    QObject::tr("Stop and reuse the quitest voice when maximum polyphony is reached"),
    QObject::tr("Stop and reuse the loudest voice when maximum polyphony is reached")
};

const QList<int> Synth::voicePolicyEnums =
{
    VP_FORGET,
    VP_LOWEST, VP_HIGHEST,
    VP_OLDEST, VP_NEWEST,
    VP_QUITEST, VP_LOUDEST
};

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
Synth::VoicePolicy Synth::voicePolicy() const { return p_->voicePolicy; }
Double Synth::volume() const { return p_->volume; }
bool Synth::combinedUnison() const { return p_->combinedUnison; }
uint Synth::unisonVoices() const { return p_->unisonVoices; }
Double Synth::unisonDetune() const { return p_->unisonDetune; }
int Synth::unisonNoteStep() const { return p_->unisonNoteStep; }

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
Double Synth::filterEnvelopeKeyFollower() const { return p_->filterEnvKeyFollow; }
Double Synth::filterAttack() const { return p_->filterAttack; }
Double Synth::filterDecay() const { return p_->filterDecay; }
Double Synth::filterSustain() const { return p_->filterSustain; }
Double Synth::filterRelease() const { return p_->filterRelease; }

void Synth::setNumberVoices(uint v) { p_->setNumVoices(v); }
void Synth::setSampleRate(uint v) { p_->sampleRate = std::max(uint(1), v); }
void Synth::setVoicePolicy(VoicePolicy v) { p_->voicePolicy = v; }
void Synth::setVolume(Double v) { p_->volume = v; }
void Synth::setCombinedUnison(bool v) { p_->combinedUnison = v; }
void Synth::setUnisonVoices(uint v) { p_->unisonVoices = std::max(uint(1), v); }
void Synth::setUnisonDetune(Double v) { p_->unisonDetune = v; }
void Synth::setUnisonNoteStep(int v) { p_->unisonNoteStep = v; }

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
void Synth::setFilterEnvelopeKeyFollower(Double v) { p_->filterEnvKeyFollow = v; }
void Synth::setFilterAttack(Double v) { p_->filterAttack = v; }
void Synth::setFilterDecay(Double v) { p_->filterDecay = v; }
void Synth::setFilterSustain(Double v) { p_->filterSustain = v; }
void Synth::setFilterRelease(Double v) { p_->filterRelease = v; }

void Synth::setVoiceStartedCallback(std::function<void (SynthVoice *)> func) { p_->cbStart_ = func; }
void Synth::setVoiceEndedCallback(std::function<void (SynthVoice *)> func) { p_->cbEnd_ = func; }



SynthVoice * Synth::noteOn(int note, Float velocity, uint startSample, void * userData)
{
    MO_DEBUG_SYNTH("Synth::noteOn(" << note << ", " << velocity << ", "
             << startSample << ", " << userData << ")");

    Double freq = p_->noteFreq.frequency(note);

    SynthVoice * voice = p_->noteOn(startSample, freq, note, velocity,
                                    p_->combinedUnison? p_->unisonVoices : 1,
                                    userData);
    if (!voice)
    {
        MO_DEBUG_SYNTH("Synth::noteOn() failed");
        return 0;
    }

    // no unisono mode?
    if (p_->unisonVoices < 2)
        return voice;

    // set frequency coefficient for combined-unisono mode
    if (p_->combinedUnison)
    {
        for (uint i=1; i<p_->unisonVoices; ++i)
        {
            note += p_->unisonNoteStep;
            freq = p_->noteFreq.frequency(note);

            // maximum (+/-) detune in Hertz
            const Double maxdetune =
                    p_->unisonDetune / 200.0
                        // range of one note
                        * (p_->noteFreq.frequency(note + 1) - freq),

                    detune = MATH::rnd(-maxdetune, maxdetune);

            voice->p_->freq = freq + detune;
            voice->p_->freq_c[i] = voice->p_->freq / sampleRate();
        }
        return voice;
    }

    const uint numUnison = std::min(numberVoices(), unisonVoices());

    // generate inidivdual voices for each unisono voice
    SynthVoice * lastv = voice;
    for (uint i=1; i<numUnison; ++i)
    {
        note += p_->unisonNoteStep;
        freq = p_->noteFreq.frequency(note);

        // maximum (+/-) detune in Hertz
        const Double maxdetune =
                p_->unisonDetune / 200.0
                    // range of one note
                    * (p_->noteFreq.frequency(note + 1) - freq);

        const Double detune = MATH::rnd(-maxdetune, maxdetune);

        SynthVoice * v = p_->noteOn(startSample, freq + detune, note, velocity, 1, userData);
        if (v)
        {
            lastv->p_->nextUnison = v;
            lastv = v;
        }
        else
        {
            MO_DEBUG_SYNTH("Synth::noteOn(): failed to created all unisono voices "
                     << i << "/" << p_->unisonVoices);
            return voice;
        }


    }

    return voice;
}

void Synth::noteOff(int note)
{
    p_->noteOff(0, note);
}

void Synth::process(F32 *output, uint bufferLength)
{
    p_->process(output, bufferLength);
}

void Synth::process(F32 ** output, uint bufferLength)
{
    p_->process(output, bufferLength);
}


} // namespace AUDIO
} // namespace MO
