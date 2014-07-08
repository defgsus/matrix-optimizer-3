/** @file sequencefloat.cpp

    @brief Float sequence

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

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

SequenceFloat::SequenceFloat(QObject *parent)
    :   Sequence    (parent),

        timeline_   (0),
        equation_   (0),

        oscMode_    (MATH::Waveform::T_SINE),

        doUseFreq_  (false),
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

    offset_ = createFloatParameter("value_offset", "value offset", 0.0);
    amplitude_ = createFloatParameter("amp", "amplitude", 1.0);
    frequency_ = createFloatParameter("freq", "frequency", 1.0);
    phase_ = createFloatParameter("phase", "phase", 0.0);
    pulseWidth_ = createFloatParameter("pulsewidth", "pulse width", 0.5);
}

void SequenceFloat::serialize(IO::DataStream &io) const
{
    Sequence::serialize(io);

    io.writeHeader("seqf", 2);

    io << sequenceTypeId[mode_];

    // osc mode
    io << MATH::Waveform::typeIds[oscMode_];

    // timeline
    io << (quint8)(timeline_ != 0);
    if (timeline_)
        timeline_->serialize(io);

    // equation (v2)
    io << equationText_ << doUseFreq_;
}

void SequenceFloat::deserialize(IO::DataStream &io)
{
    Sequence::deserialize(io);

    int ver = io.readHeader("seqf", 2);

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
    if (!equation_->parse(t.toStdString()))
        MO_WARNING("parsing failed");
}

Double SequenceFloat::value(Double time) const
{
    const Double phaseMult = 1.0 / 360.0;

    time = getSequenceTime(time);

    if (mode_ == ST_OSCILLATOR || doUseFreq_)
    {
        time = time * frequency_->value(time) + phase_->value(time) * phaseMult;
    }

    switch (mode_)
    {
        case ST_OSCILLATOR: return offset_->value(time) + amplitude_->value(time)
                * MATH::Waveform::waveform(time, oscMode_,
                        MATH::Waveform::limitPulseWidth(pulseWidth_->value(time)));

        case ST_EQUATION:
            MO_ASSERT(equation_, "SequenceFloat::value() without equation");
            // XXX NOT THREADSAFE :(
            equationTime_ = time;
            equationFreq_ = frequency_->value(time);
            equationPhase_ = phase_->value(time) * phaseMult;
            equationPW_ = MATH::Waveform::limitPulseWidth(pulseWidth_->value(time));
            return offset_->value(time) + amplitude_->value(time) * equation_->eval();

        case ST_TIMELINE: return offset_->value(time) + amplitude_->value(time) * timeline_->get(time);

        default: return offset_->value(time);
    }
}



} // namespace MO
