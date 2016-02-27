/** @file sequencefloat.cpp

    @brief Float sequence

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <random>

#include "sequencefloat.h"
#include "object/param/parameters.h"
#include "object/param/parameterfilename.h"
#include "object/param/parametertext.h"
#include "object/param/parameterint.h"
#include "object/param/parametertimeline1d.h"
#include "audio/tool/waveform.h"
#include "audio/tool/wavetablegenerator.h"
#include "audio/tool/bandlimitwavetablegenerator.h"
#include "audio/tool/fftwavetablegenerator.h"
#include "audio/tool/soundfile.h"
#include "audio/tool/soundfilemanager.h"
#include "math/timeline1d.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"


namespace MO {

/** Wrapper for equation parser to setup input variables */
class SequenceFloat::SeqEquation
{
public:

    SeqEquation()
        : equation(new PPP_NAMESPACE::Parser())
    {
        equation->variables().add("x", &time,
            SequenceFloat::tr("seconds").toStdString());
        equation->variables().add("time", &time,
            SequenceFloat::tr("seconds (same as x)").toStdString());
        equation->variables().add("xr", &rtime,
            SequenceFloat::tr("radians of time").toStdString());
        equation->variables().add("f", &freq,
            SequenceFloat::tr("the sequence frequency").toStdString());
        equation->variables().add("freq", &freq,
            SequenceFloat::tr("the sequence frequency (same as f)").toStdString());
        equation->variables().add("p", &phase,
            SequenceFloat::tr("the sequence phase").toStdString());
        equation->variables().add("phase", &phase,
            SequenceFloat::tr("the sequence phase (same as p)").toStdString());
        equation->variables().add("pw", &pw,
            SequenceFloat::tr("the sequence pulse-width").toStdString());
        equation->variables().add("pulsewidth", &pw,
            SequenceFloat::tr("the sequence pulse-width (same as pw)").toStdString());
    }

    ~SeqEquation() { delete equation; }

    PPP_NAMESPACE::Parser * equation;

    Double
        time,
        rtime,
        freq,
        phase,
        pw;
};


MO_REGISTER_OBJECT(SequenceFloat)

QStringList SequenceFloat::sequenceTypeId =
{ "c", "tl", "osc", "oscwt", "add", "addwt", "specwt", "audiof", "eq", "eqwt" };

QStringList SequenceFloat::sequenceTypeName =
{ tr("Constant"), tr("Timeline"),
  tr("Oscillator"), tr("Oscillator WT"),
  tr("Additive Osc."), tr("Additive WT"),
  tr("Spectral WT"),
  tr("Audio file"),
  tr("Equation"), tr("Equation wavetable") };

QStringList SequenceFloat::loopOverlapModeId =
{ "o", "b", "e" };

QStringList SequenceFloat::loopOverlapModeName =
{ "off", "begin", "end" };


SequenceFloat::SequenceFloat()
    :   Sequence        (),

        timeline_       (0),
        wavetable_      (0),
        soundFile_      (0),
        equation_       (0),

        p_soundFile_    (0),

        cacheValue_     (0.0),
        cacheTime_      (-1.111),
        phaseMult_      (1.0)

{
    setName("SequenceFloat");
    setNumberOutputs(ST_FLOAT, 1);
}

SequenceFloat::~SequenceFloat()
{
    delete timeline_;
    delete wavetable_;
    if (soundFile_)
        AUDIO::SoundFileManager::releaseSoundFile(soundFile_);

    for (auto e : equation_)
        delete e;
}

void SequenceFloat::createParameters()
{
    Sequence::createParameters();

    // ----- fade ------------

    params()->beginParameterGroup("fade", tr("fade in/out"));

        p_fadeIn_ = params()->createFloatParameter("fadein", tr("fade in"),
                      tr("Time in seconds to fade from zero to the actual value"),
                      0.0, 0.1);
        p_fadeIn_->setMinValue(0.0);

        p_fadeOut_ = params()->createFloatParameter("fadeout", tr("fade out"),
                      tr("Time in seconds to fade to zero at the end of the sequence"),
                      0.0, 0.1);
        p_fadeOut_->setMinValue(0.0);

        p_fadeMode_ = params()->createSelectParameter("fademode", tr("fade derivative"),
                                            tr("Selects the type of fade function"),
                                            { "lin", "quad" },
                                            { tr("linear"), tr("quadratic") },
                                            { tr("Linear fade"), tr("Smooth quadratic fade") },
                                            { 0, 1 },
                                            1, true, false);

    params()->endParameterGroup();

    // ------ value ----------

    params()->beginParameterGroup("value", tr("value"));
    initParameterGroupExpanded("value");

        p_mode_ = params()->createSelectParameter("seq_mode", tr("sequence type"),
                tr("Selects the type of the sequence content"),
                sequenceTypeId,
                sequenceTypeName,
                { tr("A constant value"),
                  tr("A editable timeline with que-points"),
                  tr("An oscillator with different waveforms"),
                  tr("A wavetable oscillator with different bandlimited waveforms"),
                  tr("A additive oscillator build from several harmonic sine tones"),
                  tr("A additive oscillator sine wavetable"),
                  tr("A spectral waveform table controlled by a frequency band curve"),
                  tr("An audio-file"),
                  tr("A mathematical equation"),
                  tr("A wavetable from a mathematical equation") },
                { ST_CONSTANT,
                  ST_TIMELINE,
                  ST_OSCILLATOR,
                  ST_OSCILLATOR_WT,
                  ST_ADD_OSC,
                  ST_ADD_WT,
                  ST_SPECTRAL_WT,
                  ST_SOUNDFILE,
                  ST_EQUATION,
                  ST_EQUATION_WT },
                  ST_CONSTANT, true, false);

        p_oscMode_ = params()->createSelectParameter("osc_mode", tr("oscillator type"),
                tr("Selects the type of the oscillator waveform"),
                AUDIO::Waveform::typeIds,
                AUDIO::Waveform::typeNames,
                AUDIO::Waveform::typeStatusTips,
                AUDIO::Waveform::typeList,
                AUDIO::Waveform::T_SINE, true, false);

        p_equationText_ = params()->createTextParameter("equ_text", tr("equation"),
                  tr("The equation is interpreted as a function of time"),
                  TT_EQUATION,
                  "sin(xr)", true, false);
        SeqEquation tmpequ;
        p_equationText_->setVariableNames(tmpequ.equation->variables().variableNames());
        p_equationText_->setVariableDescriptions(tmpequ.equation->variables().variableDescriptions());

        p_wtEquationText_ = params()->createTextParameter("equwt_text", tr("wavetable equation"),
                  tr("The equation is interpreted as a function of time and should be periodic "
                     "in the range [0,1]"),
                  TT_EQUATION,
                  "sin(xr)", true, false);
        p_wtEquationText_->setVariableNames(QStringList() << "x" << "xr");
        p_wtEquationText_->setVariableDescriptions(QStringList()
                  << tr("wavetable second [0,1]") << tr("radians of wavetable second [0,TWO_PI]"));

        p_soundFile_ = params()->createFilenameParameter("sndfilen", tr("filename"),
                                                  tr("The filename of the audio file"
                                                     " - Note that the file will be buffered in memory completely"),
                                                  IO::FT_SOUND);
        p_soundFileChannel_ = params()->createIntParameter("sndfilechan", tr("channel"),
                      tr("Selects which channel to play from the sound file"),
                      0, true, true);
        p_soundFileChannel_->setMinValue(0);
        p_soundFileChannel_->setDefaultEvolvable(false);

        p_wtSize_ = params()->createSelectParameter("wtsize", tr("wavetable size"),
                   tr("Number of samples in the wavetable"),
        { "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" },
        { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
        { "", "", "", "", "", "", "", "", "", "", "", "", "" },
        { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
        1024, true, false);
        p_wtSize_->setDefaultEvolvable(false);

        p_oscWtSize_ = params()->createSelectParameter("oscwtsize", tr("wavetable size"),
                   tr("Number of samples in the wavetable"),
        { "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" },
        { "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
        { "", "", "", "", "", "", "", "", "", "" },
        { 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
        4096, true, false);
        p_oscWtSize_->setDefaultEvolvable(false);

        p_wtFreqs_ = params()->createTimeline1DParameter("wtfreqs", tr("frequency spectrum"),
                    tr("Editable curve for the amplitude of each frequency band"),
                    0,
                    0.0, 100.0);
        p_wtPhases_ = params()->createTimeline1DParameter("wtphases", tr("phase spectrum"),
                    tr("Editable curve for the phase of each frequency band"),
                    0,
                    0.0, 100.0, -1, 1);

        p_useFreq_ = params()->createBooleanParameter("use_freq", tr("use frequency"),
                  tr("Selects whether some types of sequence which don't need a frequency "
                     "will still use it"),
                  tr("No frequency for non-oscillators"),
                  tr("Frequency is applied to non-oscillators as well"),
                  false, true, false);

        p_offset_ = params()->createFloatParameter("value_offset", tr("value offset"),
                   tr("This value is always added to the output of the sequence"),
                   0.0, 0.1);

        p_amplitude_ = params()->createFloatParameter("amp", tr("amplitude"),
                  tr("The output of the sequence (before the offset) is multiplied by this value"),
                  1.0, 0.1);

        p_frequency_ = params()->createFloatParameter("freq", tr("frequency"),
                  tr("The frequency of the function in hertz (periods per second)"),
                  1.0, 0.1);

        p_phase_ = params()->createFloatParameter("phase", tr("phase"),
                  tr("Phase (time shift) of the function, either in degree [0,360] or periods [0,1]"),
                  0.0, 0.05);

        p_doPhaseDegree_ = params()->createBooleanParameter("phase_deg", tr("phase in degree"),
                  tr("Selects whether the phase value has a range of [0,1] or [0,360]"),
                  tr("Phase is in the range of [0,1]"),
                  tr("Phase is in the range of [0,360]"),
                  false, true, false);
        p_doPhaseDegree_->setDefaultEvolvable(false);

        p_pulseWidth_ = params()->createFloatParameter("pulsewidth", tr("pulse width"),
                   tr("Pulsewidth of the waveform, describes the width of the positive edge"),
                   0.5, AUDIO::Waveform::minPulseWidth(), AUDIO::Waveform::maxPulseWidth(), 0.05);

        p_oscWtPulseWidth_ = params()->createFloatParameter("oscwtpulsewidth", tr("pulse width"),
                   tr("Pulsewidth of the waveform, describes the width of the positive edge"),
                   0.5, AUDIO::Waveform::minPulseWidth(), AUDIO::Waveform::maxPulseWidth(), 0.05,
                   true, false);

        p_smooth_ = params()->createFloatParameter("smooth", tr("smoothness"),
                   tr("A factor by wich to smooth off edges [0, 1]"),
                   0.0, 0., 1., 0.05);

    params()->endParameterGroup();

    // ---- spectral osc -----

    params()->beginParameterGroup("spectral", tr("spectral osc."));

        p_specNum_ = params()->createFloatParameter("specnum", tr("partial voices"),
                   tr("Number of partial voices of the spectral oscillator. Does not have to be an integer"),
                   8.0);
        p_specNum_->setMinValue(1.0);
        p_specNum_->setMaxValue(64.0);

        p_specOct_ = params()->createFloatParameter("specoct", tr("octave step"),
                   tr("The step in octaves between each partial voice"),
                   1.0, 0.5);

        p_specAmp_ = params()->createFloatParameter("specamp", tr("amplitude mul."),
                   tr("Multiplier for the amplitude after each partial voice"),
                   0.5, 0.1);

        p_specPhase_ = params()->createFloatParameter("specphase", tr("base phase"),
                   tr("Phase of the fundamental voice in periods [0,1] or degree [0,360]"),
                   0.0, 0.05);

        p_specPhaseShift_ = params()->createFloatParameter("specphaseshift", tr("phase shift"),
                   tr("Shift of phase per partial voice in periods [0,1] or degree [0,360]"),
                   0.0, 0.05);

        // wavetable spectral

        p_wtSpecSize_ = params()->createSelectParameter("wtspecsize", tr("wavetable size"),
                   tr("Number of samples in the wavetable"),
        { "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" },
        { "16", "32", "64", "128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536" },
        { "", "", "", "", "", "", "", "", "", "", "", "", "" },
        { 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 },
        1024, true, false);
        p_wtSpecSize_->setDefaultEvolvable(false);

        p_wtSpecNum_ = params()->createIntParameter("wtspecnum", tr("partial voices"),
                   tr("Number of partial voices of the spectral oscillator"),
                   8, 1, 128, 1, true, false);

        p_wtSpecOct_ = params()->createIntParameter("wtspecoct", tr("base octave"),
                   tr("The fundamental octave of the voice"),
                   1, 1, 128, 1, true, false);

        p_wtSpecOctStep_ = params()->createIntParameter("wtspecoctst", tr("octave step"),
                   tr("The step in octaves between each partial voice"),
                   1, 1, 128, 1, true, false);

        p_wtSpecAmp_ = params()->createFloatParameter("wtspecamp", tr("amplitude mul."),
                   tr("Additional multiplier for the amplitude after each partial voice"),
                   1.0, 0.0025, true, false);

        p_wtSpecPhase_ = params()->createFloatParameter("wtspecphase", tr("base phase"),
                   tr("Phase of the fundamental voice in periods [0,1] or degree [0,360]"),
                   0.0, 0.05, true, false);

        p_wtSpecPhaseShift_ = params()->createFloatParameter("wtspecphaseshift", tr("phase shift"),
                   tr("Shift of phase per partial voice in periods [0,1] or degree [0,360]"),
                   0.0, 0.05, true, false);

    params()->endParameterGroup();

    // ----- loop overlap ------

    params()->beginParameterGroup("loop", tr("looping"));
    params()->beginEvolveGroup(false);

        p_loopOverlapMode_ = params()->createSelectParameter("loopoverlapmode", tr("loop overlap mode"),
                tr("Selects the loop overlapping mode, that is, if and how values are mixed at "
                   "the boundaries of a loop"),
                loopOverlapModeId,
                loopOverlapModeName,
                { tr("No overlap"),
                  tr("Mix values at the beginning of the loop"),
                  tr("Mix values at the end of the loop") },
                { LOT_OFF, LOT_BEGIN, LOT_END },
                LOT_OFF, true, false);

        p_loopOverlap_ = params()->createFloatParameter("loopoverlap", tr("loop overlap"),
                   tr("Overlap of the loop window for smooth transitions (seconds)"),
                   0.1, 0.05);

        p_loopOverlapOffset_ = params()->createFloatParameter("loopoverlapofs", tr("overlap value offset"),
                   tr("A value that is added to the blended value in the transition window"),
                   0.0);

    params()->endEvolveGroup();
    params()->endParameterGroup();
}

void SequenceFloat::onParameterChanged(Parameter *p)
{
    Sequence::onParameterChanged(p);

    updateValueObjects_();
    /*
    if (p == p_mode_)
        updateValueObjects_();

    if (p == p_soundFile_)
        updateValueObjects_();
    */

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

    if (sequenceType() == ST_ADD_WT)
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

    if (sequenceType() == ST_SPECTRAL_WT)
    {
        if (p == p_wtSize_
            || p == p_wtFreqs_
            || p == p_wtPhases_)
            updateWavetable_();
    }

    if (sequenceType() == ST_EQUATION_WT)
    {
        if (p == p_wtSize_
            || p == p_wtEquationText_)
                updateWavetable_();
    }

    if (sequenceType() == ST_OSCILLATOR_WT)
    {
        if (p == p_oscWtSize_
            || p == p_oscMode_
            || p == p_oscWtPulseWidth_)
                updateWavetable_();
    }
}

void SequenceFloat::onParametersLoaded()
{
    Sequence::onParametersLoaded();

    updateValueObjects_();
    updatePhaseInDegree_();
}

void SequenceFloat::updateParameterVisibility()
{
    Sequence::updateParameterVisibility();

    const bool equ = sequenceType() == ST_EQUATION;
    const bool equwt = sequenceType() == ST_EQUATION_WT;
    const bool osc = sequenceType() == ST_OSCILLATOR;
    const bool oscwt = sequenceType() == ST_OSCILLATOR_WT;
    const bool freq = useFrequency() || typeUsesFrequency() || equ;
    const bool loop = looping();
    const bool looplap = loop && p_loopOverlapMode_->baseValue() != LOT_OFF;
    const bool pw = (osc && AUDIO::Waveform::supportsPulseWidth(oscillatorMode())) || equ;
    const bool sm = (osc && AUDIO::Waveform::supportsSmooth(oscillatorMode()));// || equ;
    const bool oscwtpw = (oscwt && AUDIO::Waveform::supportsPulseWidth(oscillatorMode()));
    const bool add = sequenceType() == ST_ADD_OSC;
    const bool addwt = sequenceType() == ST_ADD_WT;
    const bool specwt = sequenceType() == ST_SPECTRAL_WT;
    const bool wave = sequenceType() == ST_SOUNDFILE;

    p_amplitude_->setVisible(sequenceType() != ST_CONSTANT);
    p_frequency_->setVisible(freq);
    p_phase_->setVisible(freq);
    p_pulseWidth_->setVisible(pw);
    p_smooth_->setVisible(sm);
    p_loopOverlapMode_->setVisible(loop);
    p_loopOverlap_->setVisible(looplap);
    p_loopOverlapOffset_->setVisible(looplap);
    p_oscMode_->setVisible(osc || oscwt);
    p_useFreq_->setVisible(!typeUsesFrequency());
    p_doPhaseDegree_->setVisible(freq);
    p_equationText_->setVisible(equ);
    p_soundFile_->setVisible(wave);
    p_soundFileChannel_->setVisible(wave);

    p_wtFreqs_->setVisible(specwt);
    p_wtPhases_->setVisible(specwt);

    p_wtEquationText_->setVisible(equwt);
    p_wtSize_->setVisible(equwt || specwt);
    p_oscWtSize_->setVisible(oscwt);
    p_oscWtPulseWidth_->setVisible(oscwtpw);

    p_specNum_->setVisible(add);
    p_specOct_->setVisible(add);
    p_specPhase_->setVisible(add);
    p_specPhaseShift_->setVisible(add);
    p_specAmp_->setVisible(add);

    p_wtSpecSize_->setVisible(addwt);
    p_wtSpecNum_->setVisible(addwt);
    p_wtSpecOct_->setVisible(addwt);
    p_wtSpecOctStep_->setVisible(addwt);
    p_wtSpecPhase_->setVisible(addwt);
    p_wtSpecPhaseShift_->setVisible(addwt);
    p_wtSpecAmp_->setVisible(addwt);

}

void SequenceFloat::getNeededFiles(IO::FileList &files)
{
    if (sequenceType() == ST_SOUNDFILE)
    {
        files.append(IO::FileListEntry(p_soundFile_->value(), p_soundFile_->fileType()));
    }
}

void SequenceFloat::serialize(IO::DataStream &io) const
{
    Sequence::serialize(io);

    io.writeHeader("seqf", 6);

    // timeline
    io << (quint8)(timeline_ != 0);
    if (timeline_)
        timeline_->serialize(io);

}

void SequenceFloat::deserialize(IO::DataStream &io)
{
    Sequence::deserialize(io);

    int ver = io.readHeader("seqf", 6);

    if (ver <= 5)
    {
        MO_IO_ERROR(VERSION_MISMATCH, "Can't read SequenceFloat format prior to 6");
    }

    // timeline
    quint8 have;
    io >> have;
    if (have)
    {
        if (!timeline_)
            timeline_ = new MATH::Timeline1d;
        timeline_->deserialize(io);
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
    if (sequenceType() == ST_ADD_WT)
        updateWavetable_();
}

void SequenceFloat::setTimeline(const MATH::Timeline1d & tl)
{
    if (sequenceType() != ST_TIMELINE)
        setSequenceType(ST_TIMELINE);

    MO_ASSERT(timeline_, "");
    *timeline_ = tl;
}

void SequenceFloat::addTimeline(const MATH::Timeline1d & tl, Double timeOffset, bool adjustLength)
{
    if (sequenceType() != ST_TIMELINE)
        setSequenceType(ST_TIMELINE);

    MO_ASSERT(timeline_, "");
    timeline_->addTimeline(tl, timeOffset);

    if (adjustLength)
    {
        const Double
                tmin = timeline_->tmin(),
                tmax = timeline_->tmax();
        if (tmin < start())
            setStart(tmin);
        if (tmax > end())
            setEnd(tmax);
    }
}

void SequenceFloat::overwriteTimeline(const MATH::Timeline1d & tl, Double timeOffset, bool adjustLength)
{
    if (sequenceType() != ST_TIMELINE)
        setSequenceType(ST_TIMELINE);

    MO_ASSERT(timeline_, "");
    timeline_->overwriteTimeline(tl, timeOffset);

    if (adjustLength)
    {
        const Double
                tmin = timeline_->tmin(),
                tmax = timeline_->tmax();
        if (tmin < start())
            setStart(tmin);
        if (tmax > end())
            setEnd(tmax);
    }
}

void SequenceFloat::updateValueObjects_()
{
    clearError();

    // update wavetable
    if (sequenceType() == ST_ADD_WT
     || sequenceType() == ST_EQUATION_WT
     || sequenceType() == ST_OSCILLATOR_WT
     || sequenceType() == ST_SPECTRAL_WT)
    {
        if (!wavetable_)
            wavetable_ = new AUDIO::Wavetable<Double>();

        updateWavetable_();
    }    
    else
    {
        delete wavetable_;
        wavetable_ = 0;
    }

    // update timeline object
    if (sequenceType() == ST_TIMELINE)
    {
        if (!timeline_)
            timeline_ = new MATH::Timeline1d;
    }
    // ^ don't delete
    // user may want to keep the timeline

    // update equation parser
    if (sequenceType() == ST_EQUATION)
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

    // update soundfile
    if (sequenceType() == ST_SOUNDFILE && !p_soundFile_->value().isEmpty())
    {
        // remember previous for releasing
        auto oldsf = soundFile_;
        // get new
        soundFile_ = AUDIO::SoundFileManager::getSoundFile(p_soundFile_->value());
        setErrorMessage(soundFile_->errorString());
        // update loop-length default value
        if (soundFile_->isOk())
            setDefaultLoopLength(soundFile_->lengthSeconds());
        // release previous
        if (oldsf)
            oldsf->release();
    }
    else
    {
        // get rid of soundfile
        if (soundFile_)
            soundFile_->release();
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

void SequenceFloat::setSequenceType(SequenceType m)
{
    if (m != p_mode_->baseValue())
    {
        p_mode_->setValue(m);
        updateValueObjects_();
        updateParameterVisibility();
    }
}

void SequenceFloat::setEquationText(const QString & t)
{
    p_equationText_->setValue(t);

    clearError();
    const std::string text = t.toStdString();
    for (auto e : equation_)
    {
        MO_ASSERT(e, "setEquationText without equation "
                     "in SequenceFloat " << idNamePath() << " (text = '" << t << "')");
        if (!e->equation->parse(text))
        {
            MO_WARNING("parsing failed for equation in SequenceFloat '" << idName() << "'"
                       " (text = '" << t << "')");
            setErrorMessage(tr("Failed to parse equation"));
        }
    }
}

void SequenceFloat::setSoundFilename(const QString & fn)
{
    p_soundFile_->setValue(fn);
}


Double SequenceFloat::fade_(const RenderTime & time) const
{
    // fade in/out
    Double fade = 1.0;
    const Double fadein = p_fadeIn_->value(time);
    if (time.second() >= 0 && time.second() < fadein)
        fade = time.second() / fadein;
    // fadeout only for non-clip sequences
    if (!parentClip())
    {
        const Double fadeout = p_fadeOut_->value(time);
        if (fadeout > 0 && time.second() > (length() - fadeout)
                && time.second() <= length())
            fade *= (length() - time.second()) / fadeout;
    }
    // fade shape
    if (p_fadeMode_->baseValue())
        return fade * fade * (3.0 - 2.0 * fade);
    else
        return fade;
}

Double SequenceFloat::valueFloat(uint chan, const RenderTime& gtime) const
{
    if (chan != 0)
        return 0.;

    if (cacheTime_ == gtime.second())
        return cacheValue_;

    Double timeNoLoop;

    // local time
    RenderTime ltime(gtime);

    if (p_loopOverlapMode_->baseValue() == LOT_OFF)
    {
        RenderTime nlTime(ltime);
        getSequenceTime(ltime, timeNoLoop);
        nlTime += (ltime.second() - timeNoLoop);
        const Double fade = fade_(nlTime);
        return fade * value_(ltime);
    }

    bool inLoop;
    Double lStart=0, lLength=0;
    getSequenceTime(ltime, lStart, lLength, inLoop, timeNoLoop);

    const Double fade = fade_(gtime);

    //MO_PRINT(lStart << " " << lLength << " " << inLoop << " " << ltime);

    // XXX strange: inLoop comes to late, e.g. after the loop end
    if (!inLoop && p_loopOverlapMode_->baseValue() == LOT_BEGIN)
        return fade * value_(ltime);

    Double overlap = std::max(minimumLength(), p_loopOverlap_->value(ltime));

    if (p_loopOverlapMode_->baseValue() == LOT_BEGIN)
    {
        if ((ltime.second() - lStart) > overlap)
            return fade * value_(ltime);

        RenderTime nlTime(ltime);
        nlTime += (ltime.second() - timeNoLoop);
        const Double
                v = value_(ltime),
                v0 = value_(ltime + lLength)
                    + p_loopOverlapOffset_->value(ltime)
                        * p_amplitude_->value(ltime),
                t = (ltime.second() - lStart) / overlap,
                ts = t * t * (3.0 - 2.0 * t);

        return fade * ((1.0 - ts) * v0 + ts * v);
    }
    else if (p_loopOverlapMode_->baseValue() == LOT_END)
    {
        if ((ltime.second() - lStart) < lLength - overlap)
            return fade * value_(ltime);

        const Double
                v = value_(ltime),
                v0 = value_(ltime - lLength)
                    + p_loopOverlapOffset_->value(ltime)
                        * p_amplitude_->value(ltime),
                t = (lLength - ltime.second() + lStart) / overlap,
                ts = t * t * (3.0 - 2.0 * t);

        return fade * ( (1.0 - ts) * v0 + ts * v );
    }

    MO_ASSERT(false, "unhandled loopOverlapMode " << p_loopOverlapMode_->baseValue()
                     << " in SequenceFloat '" << idNamePath() << "'");
    return 0.0;
}


Double SequenceFloat::value_(const RenderTime & gtime) const
{
    RenderTime time(gtime);

    if (typeUsesFrequency() || p_useFreq_->baseValue())
    {
        time.setSecond( time.second() * p_frequency_->value(gtime)
                + p_phase_->value(gtime) * phaseMult_ );
    }

    // return value
    switch ((SequenceType)p_mode_->baseValue())
    {
        case ST_CONSTANT:
            return p_offset_->value(gtime);

        case ST_OSCILLATOR:
            return p_offset_->value(gtime) + p_amplitude_->value(gtime)
                * AUDIO::Waveform::waveform(time.second(),
                            (AUDIO::Waveform::Type)p_oscMode_->baseValue(),
                        AUDIO::Waveform::limitPulseWidth(p_pulseWidth_->value(gtime)),
                                                         p_smooth_->value(gtime));

        case ST_ADD_OSC:
            return p_offset_->value(gtime) + p_amplitude_->value(gtime)
                * AUDIO::Waveform::spectralWave(time.second(),
                                    p_specNum_->value(gtime),
                                    p_specOct_->value(gtime),
                                    p_specPhase_->value(gtime) * phaseMult_,
                                    p_specPhaseShift_->value(gtime) * phaseMult_,
                                    p_specAmp_->value(gtime)
                        );

        case ST_EQUATION_WT:
        case ST_ADD_WT:
        case ST_OSCILLATOR_WT:
        case ST_SPECTRAL_WT:
            MO_ASSERT(wavetable_, "SequenceFloat('" << idName() << "')::value() without wavetable");
            return p_offset_->value(gtime) + p_amplitude_->value(gtime)
                * wavetable_->value(time.second());

        case ST_SOUNDFILE:
            //MO_ASSERT(soundFile_, "SequenceFloat('" << idName() << "')::value() without soundfile");
            if (soundFile_)
            return p_offset_->value(gtime) + p_amplitude_->value(gtime)
                * soundFile_->value(time.second(),
                                    std::min(soundFile_->numberChannels(), (uint)p_soundFileChannel_->value(gtime)) );
            else return 0.;

        case ST_EQUATION:
            MO_ASSERT(equation_[time.thread()], "SequenceFloat('" << idName()
                        << "')::value() without equation @ " << time);
            equation_[time.thread()]->time = time.second();
            equation_[time.thread()]->rtime = time.second() * TWO_PI;
            equation_[time.thread()]->freq = p_frequency_->value(gtime);
            equation_[time.thread()]->phase = p_phase_->value(gtime) * phaseMult_;
            equation_[time.thread()]->pw = AUDIO::Waveform::limitPulseWidth(
                                        p_pulseWidth_->value(gtime));
            MO_EXTEND_EXCEPTION(
                return p_offset_->value(gtime)
                    + p_amplitude_->value(gtime) * equation_[time.thread()]->equation->eval()
                ,
                "in (thread=" << time.thread() << ") call from SequenceFloat '"
                        << name() << "' (" << idNamePath() << ")"
            );

        case ST_TIMELINE: return p_offset_->value(gtime)
                                + p_amplitude_->value(gtime)
                                    * timeline_->get(time.second());
    }

    return 0.0;
}


void SequenceFloat::getValueFloatRange(
        uint channel, const RenderTime& time, Double length,
        Double* minValue, Double* maxValue) const
{
    /** @todo need to check for changes of sequence content */
#if 1
    bool docalc = false;

    if (time.thread() >= lastMinMaxStart_.size())
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        lastMinMaxStart_.resize(time.thread()+3);
        lastMinMaxLength_.resize(time.thread()+3);
        lastMinValue_.resize(time.thread()+3);
        lastMaxValue_.resize(time.thread()+3);
        docalc = true;
    }
    else
    if (lastMinMaxStart_[time.thread()] != time.second()
            || lastMinMaxLength_[time.thread()] != length)
    {
        lastMinMaxStart_[time.thread()] = time.second();
        lastMinMaxLength_[time.thread()] = length;
        docalc = true;
    }

    if (!docalc)
    {
        *minValue = lastMinValue_[time.thread()];
        *maxValue = lastMaxValue_[time.thread()];
        return;
    }
#endif

    ValueFloatInterface::getValueFloatRange(
                channel, time, length, minValue, maxValue);

    /*
    const Double
        step = std::max(0.1, length / 5000.0);


    Double time = localStart;

    RenderTime tim(time + start(), thread);
    minValue = maxValue = valueFloat(0, tim);

    std::mt19937 rnd(12345);

    while (time < localEnd)
    {
        minValue = std::min(minValue, valueFloat(0, tim));
        maxValue = std::max(maxValue, valueFloat(0, tim));

        time += step * (1.0 + (Double)rnd()/rnd.max());
    }

    // minimum size
    if (maxValue - minValue < 0.1)
        maxValue += 0.001;
    */

    lastMinValue_[time.thread()] = *minValue;
    lastMaxValue_[time.thread()] = *maxValue;
}

void SequenceFloat::updateWavetable_()
{
    if (!wavetable_)
        abort();
    MO_ASSERT(wavetable_, "updateWavetable() without wavetable");

    if (sequenceType() == ST_OSCILLATOR_WT)
    {
        AUDIO::BandlimitWavetableGenerator gen;
        gen.setTableSize(p_oscWtSize_->baseValue());
        gen.setWaveform((AUDIO::Waveform::Type)p_oscMode_->baseValue());
        gen.setPulseWidth(p_oscWtPulseWidth_->baseValue());
        gen.createWavetable(*wavetable_);
    }

    if (sequenceType() == ST_ADD_WT)
    {
        AUDIO::WavetableGenerator gen;
        gen.setSize(p_wtSpecSize_->baseValue());
        gen.setNumPartials(p_wtSpecNum_->baseValue());
        gen.setBaseOctave(p_wtSpecOct_->baseValue());
        gen.setOctaveStep(p_wtSpecOctStep_->baseValue());
        gen.setBasePhase(p_wtSpecPhase_->baseValue() * phaseMult_);
        gen.setPhaseShift(p_wtSpecPhaseShift_->baseValue() * phaseMult_);
        gen.setAmplitudeMultiplier(p_wtSpecAmp_->baseValue());

        gen.getWavetable(wavetable_);
    }

    if (sequenceType() == ST_SPECTRAL_WT)
    {
        AUDIO::FftWavetableGenerator<Double> gen(p_wtSize_->baseValue());
        gen.setFrequencies(*p_wtFreqs_->timeline());
        gen.setPhases(*p_wtPhases_->timeline());
        gen.getWavetable(wavetable_);
    }

    if (sequenceType() == ST_EQUATION_WT)
    {
        AUDIO::BandlimitWavetableGenerator gen;

        gen.setTableSize(p_oscWtSize_->baseValue());
        gen.setMode(AUDIO::BandlimitWavetableGenerator::M_EQUATION);
        gen.setEquation(p_wtEquationText_->baseValue());
        gen.createWavetable(*wavetable_);
    }

}


} // namespace MO
