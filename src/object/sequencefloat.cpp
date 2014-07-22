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
#include "math/timeline1d.h"
#include "math/waveform.h"
#include "math/funcparser/parser.h"
#include "math/constants.h"

namespace MO {

MO_REGISTER_OBJECT(SequenceFloat)

QStringList SequenceFloat::sequenceTypeId =
{ "c", "tl", "osc", "eq" };

QStringList SequenceFloat::sequenceTypeName =
{ "Constant", "Timeline", "Oscillator", "Equation" };

QStringList SequenceFloat::loopOverlapModeId =
{ "o", "b", "e" };

QStringList SequenceFloat::loopOverlapModeName =
{ "off", "begin", "end" };


SequenceFloat::SequenceFloat(QObject *parent)
    :   Sequence    (parent),

        timeline_   (0),
        equation_   (0),

        oscMode_    (MATH::Waveform::T_SINE),
        loopOverlapMode_(LOT_OFF),

        doUseFreq_  (false),
        doPhaseDegree_(false),

        phaseMult_  (1.0),
        equationText_("sin(x*TWO_PI)")

{
    setName("SequenceFloat");
    setMode(ST_CONSTANT);
}

SequenceFloat::~SequenceFloat()
{
    if (timeline_)
        delete timeline_;

    if (equation_)
        delete equation_;
}

void SequenceFloat::createParameters()
{
    Sequence::createParameters();

    offset_ = createFloatParameter("value_offset", "value offset",
                                   tr("This value is always added to the output of the sequence"),
                                   0.0);
    amplitude_ = createFloatParameter("amp", "amplitude",
                                      tr("The output of the sequence (before the offset) is multiplied by this value"),
                                      1.0);
    frequency_ = createFloatParameter("freq", "frequency",
                                      tr("The frequency of the function in hertz (periods per second)"),
                                      1.0);
    phase_ = createFloatParameter("phase", "phase",
                                  tr("Phase (time shift) of the function, either in degree [0,360] or periods [0,1]"),
                                  0.0);
    pulseWidth_ = createFloatParameter("pulsewidth", "pulse width",
                                       tr("Pulsewidth of the waveform, describes the width of the positive edge"),
                                       0.5);
    loopOverlap_ = createFloatParameter("loopoverlap", "loop overlap",
                                       tr("Overlap of the loop window for smooth transitions (seconds)"),
                                       0.1);
    loopOverlapOffset_ = createFloatParameter("loopoverlapofs", "overlap value offset",
                                       tr("A value that is added to the blended value in the transition window"),
                                       0.0);
}

void SequenceFloat::serialize(IO::DataStream &io) const
{
    Sequence::serialize(io);

    io.writeHeader("seqf", 3);

    io << sequenceTypeId[mode_];

    // osc mode
    io << MATH::Waveform::typeIds[oscMode_];

    // timeline
    io << (quint8)(timeline_ != 0);
    if (timeline_)
        timeline_->serialize(io);

    // equation (v2)
    io << equationText_ << doUseFreq_;

    // loop overlap (v3)
    io << loopOverlapModeId[loopOverlapMode_];
}

void SequenceFloat::deserialize(IO::DataStream &io)
{
    Sequence::deserialize(io);

    int ver = io.readHeader("seqf", 3);

    if (!io.readEnum(mode_, ST_CONSTANT, sequenceTypeId))
        MO_IO_WARNING(READ, "SequenceFloat '" << idName() << "': mode not known");

    // oscillator
    if (!io.readEnum(oscMode_, MATH::Waveform::T_SINE, MATH::Waveform::typeIds))
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

    // loopoverlap (v3)
    if (ver >= 3)
    {
        if (!io.readEnum(loopOverlapMode_, LOT_OFF, loopOverlapModeId))
            MO_IO_WARNING(READ, "SequenceFloat '" << idName() << "': loop overlap mode unknown");
    }

    // create needed objects
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

    if (mode_ == ST_OSCILLATOR)
    {

    }
    else if (mode_ == ST_TIMELINE)
    {
        if (!timeline_)
            timeline_ = new MATH::Timeline1D;
    }
    else if (mode_ == ST_EQUATION)
    {
        if (!equation_)
        {
            equation_ = new PPP_NAMESPACE::Parser;
            equation_->variables().add("x", &equationTime_);
            equation_->variables().add("time", &equationTime_);
            equation_->variables().add("f", &equationFreq_);
            equation_->variables().add("freq", &equationFreq_);
            equation_->variables().add("p", &equationPhase_);
            equation_->variables().add("phase", &equationPhase_);
            equation_->variables().add("pw", &equationPW_);
            equation_->variables().add("pulsewidth", &equationPW_);

            equation_->variables().add("PI", PI);
            equation_->variables().add("TWO_PI", TWO_PI);
            equation_->variables().add("TAU", TWO_PI);
            equation_->variables().add("HALF_PI", HALF_PI);
            equation_->variables().add("E", 2.71828182845904523536);
            equation_->variables().add("PHI", (1.0 + sqrt(5.0)) / 2.0);

            equation_->parse(equationText_.toStdString().c_str());
        }
    }
}

void SequenceFloat::setEquationText(const QString & t)
{
    MO_ASSERT(equation_, "setEquationText without equation");
    equationText_ = t;
    if (!equation_->parse(equationText_.toStdString()))
        MO_WARNING("parsing failed");
}

Double SequenceFloat::value(Double gtime) const
{
    if (loopOverlapMode_ == LOT_OFF)
    {
        const Double time = getSequenceTime(gtime);
        return value_(gtime, time);
    }

    bool inLoop;
    Double lStart, lLength;
    Double time = getSequenceTime(gtime, lStart, lLength, inLoop);

    // XXX strange inLoop comes to late
    if (!inLoop && loopOverlapMode_ == LOT_BEGIN)
        return value_(gtime, time);

    Double overlap = std::max(minimumLength(), loopOverlap_->value(gtime));

    if (loopOverlapMode_ == LOT_BEGIN)
    {
        if ((time - lStart) > overlap)
            return value_(gtime, time);

        const Double
                v = value_(gtime, time),
                v0 = value_(gtime, time + lLength)
                    + loopOverlapOffset_->value(gtime) * amplitude_->value(time),
                t = (time - lStart) / overlap,
                ts = t * t * (3.0 - 2.0 * t);

        return (1.0 - ts) * v0 + ts * v;
    }
    else if (loopOverlapMode_ == LOT_END)
    {
        if ((time - lStart) < lLength - overlap)
            return value_(gtime, time);

        const Double
                v = value_(gtime, time),
                v0 = value_(gtime, time - lLength)
                    + loopOverlapOffset_->value(gtime) * amplitude_->value(time),
                t = (lLength - time + lStart) / overlap,
                ts = t * t * (3.0 - 2.0 * t);

        return (1.0 - ts) * v0 + ts * v;
    }

    MO_ASSERT(false, "unhandled loopOverlapMode " << loopOverlapMode_);
    return 0.0;
}

Double SequenceFloat::value_(Double gtime, Double time) const
{
    if (mode_ == ST_OSCILLATOR || doUseFreq_)
    {
        time = time * frequency_->value(gtime) + phase_->value(gtime) * phaseMult_;
    }

    switch (mode_)
    {
        case ST_OSCILLATOR: return offset_->value(gtime) + amplitude_->value(gtime)
                * MATH::Waveform::waveform(time, oscMode_,
                        MATH::Waveform::limitPulseWidth(pulseWidth_->value(gtime)));

        case ST_EQUATION:
            MO_ASSERT(equation_, "SequenceFloat::value() without equation");
            // XXX NOT THREADSAFE :(
            equationTime_ = time;
            equationFreq_ = frequency_->value(gtime);
            equationPhase_ = phase_->value(gtime) * phaseMult_;
            equationPW_ = MATH::Waveform::limitPulseWidth(pulseWidth_->value(gtime));
            return offset_->value(gtime) + amplitude_->value(gtime) * equation_->eval();

        case ST_TIMELINE: return offset_->value(gtime)
                                + amplitude_->value(gtime) * timeline_->get(time);

        default: return offset_->value(gtime);
    }
}


void SequenceFloat::getMinMaxValue(Double localStart, Double localEnd,
                    Double& minValue, Double& maxValue) const
{
    const Double
        len = localEnd - localStart,
        step = std::max(0.1, len / 5000.0);

    Double time = localStart;

    minValue = maxValue = value(time + start());

    std::mt19937 rnd(12345);

    while (time < localEnd)
    {
        minValue = std::min(minValue, value(time + start()));
        maxValue = std::max(maxValue, value(time + start()));

        time += step * (1.0 + (Double)rnd()/rnd.max());
    }

    // minimum size
    if (maxValue - minValue < 0.1)
        maxValue += 0.1;
}


} // namespace MO
