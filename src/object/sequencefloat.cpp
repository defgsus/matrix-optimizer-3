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

        offset_     (0.0),
        amplitude_  (1.0),

        frequency_  (1.0),
        phase_      (0.0),
        pulseWidth_ (0.5),
        oscMode_    (MATH::Waveform::T_SINE),

        equationText_("sin(x)")

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

void SequenceFloat::serialize(IO::DataStream &io) const
{
    io.writeHeader("seqf", 1);

    io << sequenceTypeId[mode_] << offset_ << amplitude_
       << frequency_ << phase_ << pulseWidth_;

    // osc mode
    io << MATH::Waveform::typeIds[oscMode_];

    // timeline
    io << (quint8)(timeline_ != 0);
    if (timeline_)
        timeline_->serialize(io);
}

void SequenceFloat::deserialize(IO::DataStream &io)
{
    io.readHeader("seqf", 1);

    if (!io.readEnum(mode_, ST_CONSTANT, sequenceTypeId))
        MO_IO_WARNING(READ, "SequenceFloat '" << idName() << "': mode not known");

    io >> offset_ >> amplitude_
       >> frequency_ >> phase_ >> pulseWidth_;

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
    if (!equation_->parse(t.toStdString().c_str()))
        MO_WARNING("parsing failed");
}

Double SequenceFloat::value(Double time) const
{
    time = getSequenceTime(time);

    const Double phaseMult = 1.0 / 360.0;

    switch (mode_)
    {
        case ST_OSCILLATOR: return offset_ + amplitude_
                * MATH::Waveform::waveform(
                        time * frequency_ + phase_ * phaseMult, oscMode_, pulseWidth_);

        case ST_EQUATION:
            MO_ASSERT(equation_, "SequenceFloat::value() without equation");
            // XXX NOT THREADSAFE :(
            equationTime_ = time * frequency_ + phase_ * phaseMult;
            return offset_ + amplitude_ * equation_->eval();

        case ST_TIMELINE: return offset_ + amplitude_ * timeline_->get(time);

        default: return offset_;
    }
}



} // namespace MO
