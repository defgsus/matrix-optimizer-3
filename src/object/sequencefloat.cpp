/** @file sequencefloat.cpp

    @brief Float sequence

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <random>

#include "sequencefloat.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "param/parameterfilename.h"
#include "param/parametertext.h"
#include "param/parameterint.h"
#include "math/timeline1d.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"
#include "audio/tool/waveform.h"
#include "audio/tool/wavetablegenerator.h"
#include "audio/tool/soundfile.h"
#include "audio/tool/soundfilemanager.h"

namespace MO {

class SequenceFloat::SeqEquation
{
public:
    SeqEquation()
        : equation(new PPP_NAMESPACE::Parser())
    {
        equation->variables().add("x", &time);
        equation->variables().add("time", &time);
        equation->variables().add("f", &freq);
        equation->variables().add("freq", &freq);
        equation->variables().add("p", &phase);
        equation->variables().add("phase", &phase);
        equation->variables().add("pw", &pw);
        equation->variables().add("pulsewidth", &pw);
    }

    ~SeqEquation() { delete equation; }

    PPP_NAMESPACE::Parser * equation;

    Double
        time,
        freq,
        phase,
        pw;
};


MO_REGISTER_OBJECT(SequenceFloat)

QStringList SequenceFloat::sequenceTypeId =
{ "c", "tl", "osc", "spec", "specwt", "audiof", "eq" };

QStringList SequenceFloat::sequenceTypeName =
{ tr("Constant"), tr("Timeline"), tr("Oscillator"),
  tr("Spectral Osc."), tr("Spectral WT"), tr("Audio file"), tr("Equation") };

QStringList SequenceFloat::loopOverlapModeId =
{ "o", "b", "e" };

QStringList SequenceFloat::loopOverlapModeName =
{ "off", "begin", "end" };


SequenceFloat::SequenceFloat(QObject *parent)
    :   Sequence        (parent),

        timeline_       (0),
        wavetable_      (0),
        wavetableGen_   (0),
        soundFile_      (0),
        equation_       (0),

        p_soundFile_    (0),

        tmp_read_pre6_  (false),

        phaseMult_      (1.0)

{
    setName("SequenceFloat");
}

SequenceFloat::~SequenceFloat()
{
    delete timeline_;
    delete wavetable_;
    delete wavetableGen_;
    if (soundFile_)
        AUDIO::SoundFileManager::releaseSoundFile(soundFile_);

    for (auto e : equation_)
        delete e;
}

void SequenceFloat::createParameters()
{
    Sequence::createParameters();

    beginParameterGroup("value", tr("value"));

        p_mode_ = createSelectParameter("seq_mode", tr("sequence type"),
                tr("Selects the type of the sequence content"),
                sequenceTypeId,
                sequenceTypeName,
                { tr("A constant value"),
                  tr("A editable timeline with que-points"),
                  tr("An oscillator with different waveforms"),
                  tr("A spectral oscillator build from several harmonic sine tones"),
                  tr("A spectral oscillator wavetable"),
                  tr("An audio-file"),
                  tr("A mathematical equation") },
                { ST_CONSTANT,
                  ST_TIMELINE,
                  ST_OSCILLATOR,
                  ST_SPECTRAL_OSC,
                  ST_SPECTRAL_WT,
                  ST_SOUNDFILE,
                  ST_EQUATION },
                  ST_CONSTANT, true, false);

        p_oscMode_ = createSelectParameter("osc_mode", tr("oscillator type"),
                tr("Selects the type of the oscillator waveform"),
                AUDIO::Waveform::typeIds,
                AUDIO::Waveform::typeNames,
                AUDIO::Waveform::typeStatusTips,
                AUDIO::Waveform::typeList,
                AUDIO::Waveform::T_SINE, true, false);

        p_equationText_ = createTextParameter("equ_text", tr("equation"),
                  tr("The equation is interpreted as a function of time"),
                  TT_EQUATION,
                  "sin(x*TWO_PI)", true, false);
        SeqEquation tmpequ;
        p_equationText_->setVariableNames(tmpequ.equation->variables().variableNames());

        p_soundFile_ = createFilenameParameter("sndfilen", tr("filename"),
                                                  tr("The filename of the audio file"),
                                                  IO::FT_SOUND_FILE);

        p_useFreq_ = createBooleanParameter("use_freq", tr("use frequency"),
                  tr("Selects whether some types of sequence which don't need a frequency "
                     "will still use it"),
                  tr("No frequency for non-oscillators"),
                  tr("Frequency is applied to non-oscillators as well"),
                  false, true, false);

        p_doPhaseDegree_ = createBooleanParameter("phase_deg", tr("phase in degree"),
                  tr("Selects whether the phase value has a range of [0,1] or [0,360]"),
                  tr("Phase is in the range of [0,1]"),
                  tr("Phase is in the range of [0,360]"),
                  false, true, false);

        p_offset_ = createFloatParameter("value_offset", tr("value offset"),
                   tr("This value is always added to the output of the sequence"),
                   0.0);

        p_amplitude_ = createFloatParameter("amp", tr("amplitude"),
                  tr("The output of the sequence (before the offset) is multiplied by this value"),
                  1.0);

        p_frequency_ = createFloatParameter("freq", tr("frequency"),
                  tr("The frequency of the function in hertz (periods per second)"),
                  1.0);

        p_phase_ = createFloatParameter("phase", tr("phase"),
                  tr("Phase (time shift) of the function, either in degree [0,360] or periods [0,1]"),
                  0.0);

        p_pulseWidth_ = createFloatParameter("pulsewidth", tr("pulse width"),
                   tr("Pulsewidth of the waveform, describes the width of the positive edge"),
                   0.5);

    endParameterGroup();


    // ---- spectral osc -----

    beginParameterGroup("spectral", tr("spectral osc."));

        p_specNum_ = createFloatParameter("specnum", tr("partial voices"),
                   tr("Number of partial voices of the spectral oscillator. Does not have to be an integer"),
                   1.0);
        p_specNum_->setMinValue(1.0);
        p_specNum_->setMaxValue(64.0);

        p_specOct_ = createFloatParameter("specoct", tr("octave step"),
                   tr("The step in octaves between each partial voice"),
                   1.0);

        p_specAmp_ = createFloatParameter("specamp", tr("amplitude mul."),
                   tr("Multiplier for the amplitude after each partial voice"),
                   0.5);

        p_specPhase_ = createFloatParameter("specphase", tr("base phase"),
                   tr("Phase of the fundamental voice in periods [0,1] or degree [0,360]"),
                   0.0);

        p_specPhaseShift_ = createFloatParameter("specphaseshift", tr("phase shift"),
                   tr("Shift of phase per partial voice in periods [0,1] or degree [0,360]"),
                   0.0);

        // wavetable spectral

        p_wtSpecSize_ = createSelectParameter("wtspecsize", tr("wavetable size"),
                   tr("Number of samples in the wavetable"),
        { "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" },
        { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
        { "", "", "", "", "", "", "", "", "", "", "", "", "" },
        { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
        1024, true, false);

        p_wtSpecNum_ = createIntParameter("wtspecnum", tr("partial voices"),
                   tr("Number of partial voices of the spectral oscillator"),
                   1, 1, 128, 1);

        p_wtSpecOct_ = createIntParameter("wtspecoct", tr("base octave"),
                   tr("The fundamental octave of the voice"),
                   1, 1, 128, 1);

        p_wtSpecOctStep_ = createIntParameter("wtspecoct", tr("octave step"),
                   tr("The step in octaves between each partial voice"),
                   1, 1, 128, 1);

        p_wtSpecAmp_ = createFloatParameter("wtspecamp", tr("amplitude mul."),
                   tr("Multiplier for the amplitude after each partial voice"),
                   0.5);

        p_wtSpecPhase_ = createFloatParameter("wtspecphase", tr("base phase"),
                   tr("Phase of the fundamental voice in periods [0,1] or degree [0,360]"),
                   0.0);

        p_wtSpecPhaseShift_ = createFloatParameter("wtspecphaseshift", tr("phase shift"),
                   tr("Shift of phase per partial voice in periods [0,1] or degree [0,360]"),
                   0.0);

    endParameterGroup();

    // ----- loop overlap ------

    beginParameterGroup("loop", tr("looping"));

        p_loopOverlapMode_ = createSelectParameter("loopoverlapmode", tr("loop overlap mode"),
                tr("Selects the loop overlapping mode, that is, if and how values are mixed at "
                   "the edge of a loop"),
                loopOverlapModeId,
                loopOverlapModeName,
                { tr("No overlap"),
                  tr("Mix values at the beginning of the loop"),
                  tr("Mix values at the end of the loop") },
                { LOT_OFF, LOT_BEGIN, LOT_END },
                LOT_OFF, true, false);

        p_loopOverlap_ = createFloatParameter("loopoverlap", tr("loop overlap"),
                   tr("Overlap of the loop window for smooth transitions (seconds)"),
                   0.1);

        p_loopOverlapOffset_ = createFloatParameter("loopoverlapofs", tr("overlap value offset"),
                   tr("A value that is added to the blended value in the transition window"),
                   0.0);

    endParameterGroup();
}

void SequenceFloat::onParameterChanged(Parameter *p)
{
    Sequence::onParameterChanged(p);

    if (p == p_mode_)
        updateValueObjects_();

    if (p == p_soundFile_)
        updateValueObjects_();

    if (p == p_doPhaseDegree_)
        updatePhaseInDegree_();

    if (p == p_equationText_)
    {
        const std::string text = p_equationText_->baseValue().toStdString();
        for (auto e : equation_)
        {
            MO_ASSERT(e, "setEquationText without equation "
                         "in SequenceFloat " << idNamePath() << " (text = '" << text << "')");
            if (!e->equation->parse(text))
                MO_WARNING("parsing failed for equation in SequenceFloat '" << idName() << "'"
                           " (text = '" << text << "')");
        }
    }

    if (sequenceType() == ST_SPECTRAL_WT)
    {
        if (p == p_wtSpecSize_
            || p == p_wtSpecNum_
            || p == p_wtSpecOct_
            || p == p_wtSpecOctStep_
            || p == p_wtSpecPhase_
            || p == p_wtSpecPhaseShift_
            || p == p_wtSpecAmp_)
                updateWavetable_();
    }
}

void SequenceFloat::onParametersLoaded()
{
    Sequence::onParametersLoaded();

    if (tmp_read_pre6_)
    {
        tmp_read_pre6_ = false;
        p_mode_->setValue(tmp_mode_);
        p_oscMode_->setValue(tmp_oscMode_);
        p_useFreq_->setValue(tmp_doUseFreq_);
        p_doPhaseDegree_->setValue(tmp_doPhaseDegree_);
        p_loopOverlapMode_->setValue(tmp_loopOverlapMode_);
        p_equationText_->setValue(tmp_equationText_);

        if (sequenceType() == ST_EQUATION && wavetableGen_)
        {
            p_wtSpecSize_->setValue(wavetableGen_->size());
            p_wtSpecNum_->setValue(wavetableGen_->numPartials());
            p_wtSpecOct_->setValue(wavetableGen_->baseOctave());
            p_wtSpecOctStep_->setValue(wavetableGen_->octaveStep());
            p_wtSpecPhase_->setValue(wavetableGen_->basePhase());
            p_wtSpecPhaseShift_->setValue(wavetableGen_->phaseShift());
            p_wtSpecAmp_->setValue(wavetableGen_->amplitudeMultiplier());
        }
    }

    updateValueObjects_();
    updatePhaseInDegree_();
}

void SequenceFloat::updateParameterVisibility()
{
    Sequence::updateParameterVisibility();

    const bool equ = sequenceType() == ST_EQUATION;
    const bool freq = useFrequency() || typeUsesFrequency() || equ;
    const bool loop = looping();
    const bool looplap = loop && p_loopOverlap_->baseValue() != LOT_OFF;
    const bool pw = AUDIO::Waveform::supportsPulseWidth(oscillatorMode()) || equ;
    const bool spec = sequenceType() == ST_SPECTRAL_OSC;
    const bool wtspec = sequenceType() == ST_SPECTRAL_WT;
    const bool wave = sequenceType() == ST_SOUNDFILE;

    p_amplitude_->setVisible(sequenceType() != ST_CONSTANT);
    p_frequency_->setVisible(freq);
    p_phase_->setVisible(freq);
    p_pulseWidth_->setVisible(pw);
    p_loopOverlapMode_->setVisible(loop);
    p_loopOverlap_->setVisible(looplap);
    p_loopOverlapOffset_->setVisible(looplap);
    p_oscMode_->setVisible(sequenceType() == ST_OSCILLATOR);
    p_useFreq_->setVisible(!typeUsesFrequency());
    p_doPhaseDegree_->setVisible(freq);
    p_equationText_->setVisible(equ);
    p_soundFile_->setVisible(wave);

    p_specNum_->setVisible(spec);
    p_specOct_->setVisible(spec);
    p_specPhase_->setVisible(spec);
    p_specPhaseShift_->setVisible(spec);
    p_specAmp_->setVisible(spec);

    p_wtSpecSize_->setVisible(wtspec);
    p_wtSpecNum_->setVisible(wtspec);
    p_wtSpecOct_->setVisible(wtspec);
    p_wtSpecOctStep_->setVisible(wtspec);
    p_wtSpecPhase_->setVisible(wtspec);
    p_wtSpecPhaseShift_->setVisible(wtspec);
    p_wtSpecAmp_->setVisible(wtspec);

}

void SequenceFloat::serialize(IO::DataStream &io) const
{
    Sequence::serialize(io);

    io.writeHeader("seqf", 6);

#if (0) // PRE_6_VERSION
    io << sequenceTypeId[mode_];

    // osc mode
    io << AUDIO::Waveform::typeIds[oscMode_];
#endif
    // timeline
    io << (quint8)(timeline_ != 0);
    if (timeline_)
        timeline_->serialize(io);
#if (0) // PRE_6_VERSION
    // equation (v2)
    io << equationText_ << doUseFreq_;

    // v5
    io << doPhaseDegree_;

    // loop overlap (v3)
    io << loopOverlapModeId[loopOverlapMode_];

    // wavetablegen (v4)
    io << (qint8)(wavetableGen_ != 0);
    if (wavetableGen_)
        wavetableGen_->serialize(io);
#endif

}

void SequenceFloat::deserialize(IO::DataStream &io)
{
    Sequence::deserialize(io);

    int ver = io.readHeader("seqf", 6);

    if (ver <= 5)
    {
        tmp_read_pre6_ = true;

        tmp_doUseFreq_ = false;
        tmp_doPhaseDegree_ = false;
        tmp_oscMode_ = AUDIO::Waveform::T_SINE;
        tmp_mode_ = ST_CONSTANT;
        tmp_equationText_ = "sin(x*TWO_PI)";

        if (!io.readEnum(tmp_mode_, ST_CONSTANT, sequenceTypeId))
            MO_IO_WARNING(READ, "SequenceFloat '" << idName() << "': mode not known");

        // oscillator
        if (!io.readEnum(tmp_oscMode_, AUDIO::Waveform::T_SINE, AUDIO::Waveform::typeIds))
            MO_IO_WARNING(READ, "SequenceFloat '" << idName() << "': oscillator mode not known");
    }

    // timeline
    quint8 have;
    io >> have;
    if (have)
    {
        if (!timeline_)
            timeline_ = new MATH::Timeline1D;
        timeline_->deserialize(io);
    }

    if (ver <= 5)
    {
        // equation (v2)
        if (ver >= 2)
            io >> tmp_equationText_ >> tmp_doUseFreq_;

        if (ver >= 5)
        {
            io >> tmp_doPhaseDegree_;
        }

        // loopoverlap (v3)
        if (ver >= 3)
        {
            if (!io.readEnum(tmp_loopOverlapMode_, LOT_OFF, loopOverlapModeId))
                MO_IO_WARNING(READ, "SequenceFloat '" << idName() << "': loop overlap mode unknown");
        }

        // wavetable generator
        if (ver >= 4)
        {
            qint8 isWg;
            io >> isWg;
            if (isWg)
            {
                if (!wavetableGen_)
                    wavetableGen_ = new AUDIO::WavetableGenerator();
                wavetableGen_->deserialize(io);
            }
        }
    }
}

const QString& SequenceFloat::equationText() const
{
    return p_equationText_->baseValue();
}

void SequenceFloat::setNumberThreads(uint num)
{
    Sequence::setNumberThreads(num);

    uint oldnum = equation_.size();
    equation_.resize(num);

    // XXX not safe in all cases
    // (but right now setNumberThreads is only called once)
    for (uint i=oldnum; i<num; ++i)
        equation_[i] = 0;

    // update/create equation objects
    updateValueObjects_();
}

void SequenceFloat::updatePhaseInDegree_()
{
    phaseMult_ = p_doPhaseDegree_->baseValue()? 1.0 / 360.0 : 1.0;
    if (sequenceType() == ST_SPECTRAL_WT)
        updateWavetable_();
}

void SequenceFloat::updateValueObjects_()
{
    if (sequenceType() == ST_SPECTRAL_WT)
    {
        if (!wavetable_)
            wavetable_ = new AUDIO::Wavetable<Double>();
        if (!wavetableGen_)
            wavetableGen_ = new AUDIO::WavetableGenerator();
        updateWavetable_();
    }
    else
    {
        if (wavetable_)
            delete wavetable_;
        wavetable_ = 0;
    }

    if (p_mode_->baseValue() == ST_TIMELINE)
    {
        if (!timeline_)
            timeline_ = new MATH::Timeline1D;
    }

    if (p_mode_->baseValue() == ST_EQUATION)
    {
        for (auto &e : equation_)
        if (!e)
        {
            e = new SeqEquation();
            e->equation->parse(p_equationText_->value().toStdString());
        }
    }
    else
    {
        for (auto &e : equation_)
        {
            delete e;
            e = 0;
        }
    }

    if (p_mode_->baseValue() == ST_SOUNDFILE)
    {
        // release previous, if filename changed
        if (soundFile_ && soundFile_->filename() != p_soundFile_->value())
            AUDIO::SoundFileManager::releaseSoundFile(soundFile_);
        // get new
        soundFile_ = AUDIO::SoundFileManager::getSoundFile(p_soundFile_->value());
    }
    else
    {
        // get rid of soundfile
        if (soundFile_)
            AUDIO::SoundFileManager::releaseSoundFile(soundFile_);
        soundFile_ = 0;
    }
}

PPP_NAMESPACE::Parser * SequenceFloat::equation(uint thread)
{
    return equation_[thread] ? equation_[thread]->equation : 0;
}


const PPP_NAMESPACE::Parser * SequenceFloat::equation(uint thread) const
{
    return equation_[thread] ? equation_[thread]->equation : 0;
}

void SequenceFloat::setEquationText(const QString & t)
{
    p_equationText_->setValue(t);

    const std::string text = t.toStdString();
    for (auto e : equation_)
    {
        MO_ASSERT(e, "setEquationText without equation "
                     "in SequenceFloat " << idNamePath() << " (text = '" << t << "')");
        if (!e->equation->parse(text))
            MO_WARNING("parsing failed for equation in SequenceFloat '" << idName() << "'"
                       " (text = '" << t << "')");
    }
}

Double SequenceFloat::value(Double gtime, uint thread) const
{
    if (p_loopOverlapMode_->baseValue() == LOT_OFF)
    {
        const Double time = getSequenceTime(gtime, thread);
        return value_(gtime, time, thread);
    }

    bool inLoop;
    Double lStart, lLength;
    Double time = getSequenceTime(gtime, thread, lStart, lLength, inLoop);

    // XXX strange: inLoop comes to late, e.g. after the loop end
    if (!inLoop && p_loopOverlapMode_->baseValue() == LOT_BEGIN)
        return value_(gtime, time, thread);

    Double overlap = std::max(minimumLength(), p_loopOverlap_->value(gtime, thread));

    if (p_loopOverlapMode_->baseValue() == LOT_BEGIN)
    {
        if ((time - lStart) > overlap)
            return value_(gtime, time, thread);

        const Double
                v = value_(gtime, time, thread),
                v0 = value_(gtime, time + lLength, thread)
                    + p_loopOverlapOffset_->value(gtime, thread)
                        * p_amplitude_->value(time, thread),
                t = (time - lStart) / overlap,
                ts = t * t * (3.0 - 2.0 * t);

        return (1.0 - ts) * v0 + ts * v;
    }
    else if (p_loopOverlapMode_->baseValue() == LOT_END)
    {
        if ((time - lStart) < lLength - overlap)
            return value_(gtime, time, thread);

        const Double
                v = value_(gtime, time, thread),
                v0 = value_(gtime, time - lLength, thread)
                    + p_loopOverlapOffset_->value(gtime, thread)
                        * p_amplitude_->value(time, thread),
                t = (lLength - time + lStart) / overlap,
                ts = t * t * (3.0 - 2.0 * t);

        return (1.0 - ts) * v0 + ts * v;
    }

    MO_ASSERT(false, "unhandled loopOverlapMode " << p_loopOverlapMode_->baseValue()
                     << " in SequenceFloat '" << idNamePath() << "'");
    return 0.0;
}


Double SequenceFloat::value_(Double gtime, Double time, uint thread) const
{
    if (typeUsesFrequency() || p_useFreq_->baseValue())
    {
        time = time * p_frequency_->value(gtime, thread)
                + p_phase_->value(gtime, thread) * phaseMult_;
    }

    switch ((SequenceType)p_mode_->baseValue())
    {
        case ST_OSCILLATOR:
            return p_offset_->value(gtime, thread) + p_amplitude_->value(gtime, thread)
                * AUDIO::Waveform::waveform(time,
                            (AUDIO::Waveform::Type)p_oscMode_->baseValue(),
                        AUDIO::Waveform::limitPulseWidth(p_pulseWidth_->value(gtime, thread)));

        case ST_SPECTRAL_OSC:
            return p_offset_->value(gtime, thread) + p_amplitude_->value(gtime, thread)
                * AUDIO::Waveform::spectralWave(time,
                                    p_specNum_->value(gtime, thread),
                                    p_specOct_->value(gtime, thread),
                                    p_specPhase_->value(gtime, thread) * phaseMult_,
                                    p_specPhaseShift_->value(gtime, thread) * phaseMult_,
                                    p_specAmp_->value(gtime, thread)
                        );


        case ST_SPECTRAL_WT:
            MO_ASSERT(wavetable_, "SequenceFloat('" << idName() << "')::value() without wavetable");
            return p_offset_->value(gtime, thread) + p_amplitude_->value(gtime, thread)
                * wavetable_->value(time);

        case ST_SOUNDFILE:
            MO_ASSERT(soundFile_, "SequenceFloat('" << idName() << "')::value() without soundfile");
            return p_offset_->value(gtime, thread) + p_amplitude_->value(gtime, thread)
                * soundFile_->value(time);

        case ST_EQUATION:
            MO_ASSERT(equation_[thread], "SequenceFloat('" << idName() << "')::value() without equation "
                                         "(thread=" << thread << ")");
            equation_[thread]->time = time;
            equation_[thread]->freq = p_frequency_->value(gtime, thread);
            equation_[thread]->phase = p_phase_->value(gtime, thread) * phaseMult_;
            equation_[thread]->pw = AUDIO::Waveform::limitPulseWidth(
                                        p_pulseWidth_->value(gtime, thread));
            MO_EXTEND_EXCEPTION(
                return p_offset_->value(gtime, thread)
                    + p_amplitude_->value(gtime, thread) * equation_[thread]->equation->eval()
                ,
                "in (thread=" << thread << ") call from SequenceFloat '"
                        << name() << "' (" << idNamePath() << ")"
            );

        case ST_TIMELINE: return p_offset_->value(gtime, thread)
                                + p_amplitude_->value(gtime, thread)
                                    * timeline_->get(time);

        default: return p_offset_->value(gtime, thread);
    }
}


void SequenceFloat::getMinMaxValue(Double localStart, Double localEnd,
                    Double& minValue, Double& maxValue, uint thread) const
{
    const Double
        len = localEnd - localStart,
        step = std::max(0.1, len / 5000.0);

    Double time = localStart;

    minValue = maxValue = value(time + start(), thread);

    std::mt19937 rnd(12345);

    while (time < localEnd)
    {
        minValue = std::min(minValue, value(time + start(), thread));
        maxValue = std::max(maxValue, value(time + start(), thread));

        time += step * (1.0 + (Double)rnd()/rnd.max());
    }

    // minimum size
    if (maxValue - minValue < 0.1)
        maxValue += 0.1;
}

void SequenceFloat::updateWavetable_()
{
    MO_ASSERT(wavetable_ && wavetableGen_, "updateWavetable() without wavetable");

    wavetableGen_->setSize(p_wtSpecSize_->baseValue());
    wavetableGen_->setNumPartials(p_wtSpecNum_->baseValue());
    wavetableGen_->setBaseOctave(p_wtSpecOct_->baseValue());
    wavetableGen_->setOctaveStep(p_wtSpecOctStep_->baseValue());
    wavetableGen_->setBasePhase(p_wtSpecPhase_->baseValue() * phaseMult_);
    wavetableGen_->setPhaseShift(p_wtSpecPhaseShift_->baseValue() * phaseMult_);
    wavetableGen_->setAmplitudeMultiplier(p_wtSpecAmp_->baseValue());

    wavetableGen_->getWavetable(wavetable_);
}


} // namespace MO
