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

namespace MO {

MO_REGISTER_OBJECT(SequenceFloat)

QStringList SequenceFloat::sequenceTypeId =
{ "c", "tl", "osc", "eq" };

QStringList SequenceFloat::sequenceTypeName =
{ "Constant", "Timeline", "Oscillator", "Equation" };

SequenceFloat::SequenceFloat(QObject *parent)
    :   Sequence    (parent),
        timeline_   (0)
{
    setName("SequenceFloat");
    setMode(ST_CONSTANT);
}


void SequenceFloat::serialize(IO::DataStream &io) const
{
    io.writeHeader("seqf", 1);

    io << sequenceTypeId[mode_];

    // contains osc
    io << (quint8)0;

    // timeline
    io << (quint8)(timeline_ != 0);
    if (timeline_)
        timeline_->serialize(io);
}

void SequenceFloat::deserialize(IO::DataStream &io)
{
    io.readHeader("seqf", 1);

    QString modeStr;
    io >> modeStr;

    if (!sequenceTypeId.contains(modeStr))
    {
        MO_IO_WARNING(READ, "SequenceFloat type '" << modeStr << "' not known");
        mode_ = ST_OSCILLATOR;
    }
    else
        mode_ = (SequenceType)sequenceTypeId.indexOf(modeStr);



    // oscillator
    quint8 have;
    io >> have;

    // timeline
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
    switch (mode_)
    {
        case ST_OSCILLATOR: return sin(time);
        case ST_CONSTANT: return 0;
        case ST_EQUATION: return 0;
        case ST_TIMELINE: return timeline_->get(time);
        default: return 0;
    }
}



} // namespace MO
