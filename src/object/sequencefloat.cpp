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

namespace MO {

MO_REGISTER_OBJECT(SequenceFloat)

QStringList SequenceFloat::sequenceTypeId =
{ "c", "tl", "osc", "eq" };

QStringList SequenceFloat::sequenceTypeName =
{ "Constant", "Timeline", "Oscillator", "Equation" };

SequenceFloat::SequenceFloat(QObject *parent)
    :   Sequence    (parent),

        timeline_   (0),
        offset_     (0.0),
        amplitude_  (1.0),

        frequency_  (1.0),
        phase_      (0.0),
        pulseWidth_ (0.5),
        oscMode_    (MATH::Waveform::T_SINE)
{
    setName("SequenceFloat");
    setMode(ST_CONSTANT);
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

    }
}

Double SequenceFloat::value(Double time) const
{
    time = getSequenceTime(time);

    switch (mode_)
    {
        case ST_OSCILLATOR: return offset_ + amplitude_
                * MATH::Waveform::waveform(time * frequency_ + phase_, oscMode_, pulseWidth_);

        case ST_EQUATION: return offset_ + amplitude_ * 0.0;

        case ST_TIMELINE: return offset_ + amplitude_ * timeline_->get(time);

        default: return offset_;
    }
}



} // namespace MO
