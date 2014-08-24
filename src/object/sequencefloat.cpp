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
#include "param/parameterfilename.h"
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

        paramSoundFile_ (0),

        oscMode_        (AUDIO::Waveform::T_SINE),
        loopOverlapMode_(LOT_OFF),

        doUseFreq_      (false),
        doPhaseDegree_  (false),

        phaseMult_      (1.0),
        equationText_   ("sin(x*TWO_PI)")

{
    setName("SequenceFloat");
    setMode(ST_CONSTANT);
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

        offset_ = createFloatParameter("value_offset", "value offset",
                   tr("This value is always added to the output of the sequence"),
                   0.0);
        offset_->setEditable(false);

        amplitude_ = createFloatParameter("amp", "amplitude",
                  tr("The output of the sequence (before the offset) is multiplied by this value"),
                  1.0);
        amplitude_->setEditable(false);

        frequency_ = createFloatParameter("freq", "frequency",
                  tr("The frequency of the function in hertz (periods per second)"),
                  1.0);
        frequency_->setEditable(false);

        phase_ = createFloatParameter("phase", "phase",
                  tr("Phase (time shift) of the function, either in degree [0,360] or periods [0,1]"),
                  0.0);
        phase_->setEditable(false);

        pulseWidth_ = createFloatParameter("pulsewidth", "pulse width",
                   tr("Pulsewidth of the waveform, describes the width of the positive edge"),
                   0.5);
        pulseWidth_->setEditable(false);

    endParameterGroup();

    // --- audio file ---

    beginParameterGroup("sndfile", tr("audio file"));

        paramSoundFile_ = createFilenameParameter("sndfilen", tr("filename"),
                                                  tr("The filename of the audio file"),
                                                  IO::FT_SOUND_FILE);

    endParameterGroup();

    // ---- spectral osc -----

    beginParameterGroup("spectral", tr("spectral osc."));

        specNum_ = createFloatParameter("specnum", "partial voices",
                   tr("Number of partial voices of the spectral oscillator. Does not have to be an integer"),
                   1.0);
        specNum_->setEditable(false);
        specNum_->setMinValue(1.0);
        specNum_->setMaxValue(64.0);

        specOct_ = createFloatParameter("specoct", "octave step",
                   tr("The step in octaves between each partial voice"),
                   1.0);
        specOct_->setEditable(false);

        specAmp_ = createFloatParameter("specamp", "amplitude mul.",
                   tr("Multiplier for the amplitude after each partial voice"),
                   0.5);
        specAmp_->setEditable(false);

        specPhase_ = createFloatParameter("specphase", "base phase",
                   tr("Phase of the fundamental voice in periods [0,1] or degree [0,360]"),
                   0.0);
        specPhase_->setEditable(false);

        specPhaseShift_ = createFloatParameter("specphaseshift", "phase shift",
                   tr("Shift of phase per partial voice in periods [0,1] or degree [0,360]"),
                   0.0);
        specPhaseShift_->setEditable(false);

    endParameterGroup();

    // ----- loop overlap ------

    beginParameterGroup("loopoverlap", tr("loop overlap"));

        loopOverlap_ = createFloatParameter("loopoverlap", "loop overlap",
                   tr("Overlap of the loop window for smooth transitions (seconds)"),
                   0.1);
        loopOverlap_->setEditable(false);

        loopOverlapOffset_ = createFloatParameter("loopoverlapofs", "overlap value offset",
                   tr("A value that is added to the blended value in the transition window"),
                   0.0);
        loopOverlapOffset_->setEditable(false);

    endParameterGroup();
}

void SequenceFloat::onParameterChanged(Parameter *p)
{
    Sequence::onParameterChanged(p);

    if (p == paramSoundFile_)
        setMode(mode_);
}

void SequenceFloat::serialize(IO::DataStream &io) const
{
    Sequence::serialize(io);

    io.writeHeader("seqf", 5);

    io << sequenceTypeId[mode_];

    // osc mode
    io << AUDIO::Waveform::typeIds[oscMode_];

    // timeline
    io << (quint8)(timeline_ != 0);
    if (timeline_)
        timeline_->serialize(io);

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

}

void SequenceFloat::deserialize(IO::DataStream &io)
{
    Sequence::deserialize(io);

    int ver = io.readHeader("seqf", 5);

    if (!io.readEnum(mode_, ST_CONSTANT, sequenceTypeId))
        MO_IO_WARNING(READ, "SequenceFloat '" << idName() << "': mode not known");

    // oscillator
    if (!io.readEnum(oscMode_, AUDIO::Waveform::T_SINE, AUDIO::Waveform::typeIds))
        MO_IO_WARNING(READ, "SequenceFloat '" << idName() << "': oscillator mode not known");

    // timeline
    quint8 have;
    io >> have;
    if (have)
    {
        if (!timeline_)
            timeline_ = new MATH::Timeline1D;
        timeline_->deserialize(io);
    }

    // equation (v2)
    if (ver >= 2)
        io >> equationText_ >> doUseFreq_;

    if (ver >= 5)
    {
        io >> doPhaseDegree_;
        setPhaseInDegree(doPhaseDegree_);
    }

    // loopoverlap (v3)
    if (ver >= 3)
    {
        if (!io.readEnum(loopOverlapMode_, LOT_OFF, loopOverlapModeId))
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

    // create needed objects
    setMode(mode_);
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
    setMode(mode_);
}

void SequenceFloat::setPhaseInDegree(bool enable)
{
    doPhaseDegree_ = enable;
    phaseMult_ = enable? 1.0 / 360.0 : 1.0;
}

void SequenceFloat::setMode(SequenceType m)
{
    mode_ = m;


    if (mode_ == ST_SPECTRAL_WT)
    {
        if (!wavetable_)
            wavetable_ = new AUDIO::Wavetable<Double>();
        if (!wavetableGen_)
            wavetableGen_ = new AUDIO::WavetableGenerator();
        updateWavetable();
    }
    else
    {
        if (wavetable_)
            delete wavetable_;
        wavetable_ = 0;
    }

    if (mode_ == ST_TIMELINE)
    {
        if (!timeline_)
            timeline_ = new MATH::Timeline1D;
    }

    if (mode_ == ST_EQUATION)
    {
        for (auto &e : equation_)
        if (!e)
        {
            e = new SeqEquation();
            e->equation->parse(equationText_.toStdString());
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

    if (mode_ == ST_SOUNDFILE && paramSoundFile_)
    {
        // release previous, if filename changed
        if (soundFile_ && soundFile_->filename() != paramSoundFile_->value())
            AUDIO::SoundFileManager::releaseSoundFile(soundFile_);
        // get new
        soundFile_ = AUDIO::SoundFileManager::getSoundFile(paramSoundFile_->value());
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
    equationText_ = t;
    const std::string text = equationText_.toStdString();
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
    if (loopOverlapMode_ == LOT_OFF)
    {
        const Double time = getSequenceTime(gtime, thread);
        return value_(gtime, time, thread);
    }

    bool inLoop;
    Double lStart, lLength;
    Double time = getSequenceTime(gtime, thread, lStart, lLength, inLoop);

    // XXX strange: inLoop comes to late, e.g. after the loop end
    if (!inLoop && loopOverlapMode_ == LOT_BEGIN)
        return value_(gtime, time, thread);

    Double overlap = std::max(minimumLength(), loopOverlap_->value(gtime, thread));

    if (loopOverlapMode_ == LOT_BEGIN)
    {
        if ((time - lStart) > overlap)
            return value_(gtime, time, thread);

        const Double
                v = value_(gtime, time, thread),
                v0 = value_(gtime, time + lLength, thread)
                    + loopOverlapOffset_->value(gtime, thread)
                        * amplitude_->value(time, thread),
                t = (time - lStart) / overlap,
                ts = t * t * (3.0 - 2.0 * t);

        return (1.0 - ts) * v0 + ts * v;
    }
    else if (loopOverlapMode_ == LOT_END)
    {
        if ((time - lStart) < lLength - overlap)
            return value_(gtime, time, thread);

        const Double
                v = value_(gtime, time, thread),
                v0 = value_(gtime, time - lLength, thread)
                    + loopOverlapOffset_->value(gtime, thread)
                        * amplitude_->value(time, thread),
                t = (lLength - time + lStart) / overlap,
                ts = t * t * (3.0 - 2.0 * t);

        return (1.0 - ts) * v0 + ts * v;
    }

    MO_ASSERT(false, "unhandled loopOverlapMode " << loopOverlapMode_
                     << " in SequenceFloat '" << idNamePath() << "'");
    return 0.0;
}


Double SequenceFloat::value_(Double gtime, Double time, uint thread) const
{
    if (typeUsesFrequency() || doUseFreq_)
    {
        time = time * frequency_->value(gtime, thread)
                + phase_->value(gtime, thread) * phaseMult_;
    }

    switch (mode_)
    {
        case ST_OSCILLATOR:
            return offset_->value(gtime, thread) + amplitude_->value(gtime, thread)
                * AUDIO::Waveform::waveform(time, oscMode_,
                        AUDIO::Waveform::limitPulseWidth(pulseWidth_->value(gtime, thread)));

        case ST_SPECTRAL_OSC:
            return offset_->value(gtime, thread) + amplitude_->value(gtime, thread)
                * AUDIO::Waveform::spectralWave(time,
                                    specNum_->value(gtime, thread),
                                    specOct_->value(gtime, thread),
                                    specPhase_->value(gtime, thread) * phaseMult_,
                                    specPhaseShift_->value(gtime, thread) * phaseMult_,
                                    specAmp_->value(gtime, thread)
                        );


        case ST_SPECTRAL_WT:
            MO_ASSERT(wavetable_, "SequenceFloat('" << idName() << "')::value() without wavetable");
            return offset_->value(gtime, thread) + amplitude_->value(gtime, thread)
                * wavetable_->value(time);

        case ST_SOUNDFILE:
            MO_ASSERT(soundFile_, "SequenceFloat('" << idName() << "')::value() without soundfile");
            return offset_->value(gtime, thread) + amplitude_->value(gtime, thread)
                * soundFile_->value(time);

        case ST_EQUATION:
            MO_ASSERT(equation_[thread], "SequenceFloat('" << idName() << "')::value() without equation "
                                         "(thread=" << thread << ")");
            equation_[thread]->time = time;
            equation_[thread]->freq = frequency_->value(gtime, thread);
            equation_[thread]->phase = phase_->value(gtime, thread) * phaseMult_;
            equation_[thread]->pw = AUDIO::Waveform::limitPulseWidth(
                                        pulseWidth_->value(gtime, thread));
            MO_EXTEND_EXCEPTION(
                return offset_->value(gtime, thread)
                    + amplitude_->value(gtime, thread) * equation_[thread]->equation->eval()
                ,
                "in (thread=" << thread << ") call from SequenceFloat '"
                        << name() << "' (" << idNamePath() << ")"
            );

        case ST_TIMELINE: return offset_->value(gtime, thread)
                                + amplitude_->value(gtime, thread)
                                    * timeline_->get(time);

        default: return offset_->value(gtime, thread);
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

void SequenceFloat::updateWavetable()
{
    MO_ASSERT(wavetable_ && wavetableGen_, "updateWavetable() without wavetable");

    wavetableGen_->getWavetable(wavetable_);
}


} // namespace MO
