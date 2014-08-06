/** @file modulatorfloat.cpp

    @brief Float modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "modulatorfloat.h"
#include "object/trackfloat.h"
#include "object/sequencefloat.h"
#include "io/error.h"
#include "io/datastream.h"


namespace MO {


ModulatorFloat::ModulatorFloat(const QString &modulatorId, Object *parent)
    : Modulator     (modulatorId, parent),
      sourceType_   (ST_NONE)
{
}

void ModulatorFloat::serialize(IO::DataStream & io) const
{
    Modulator::serialize(io);

    io.writeHeader("modf", 1);

    io << amplitude_ << timeOffset_;
}

void ModulatorFloat::deserialize(IO::DataStream & io)
{
    Modulator::deserialize(io);

    io.readHeader("modf", 1);

    io >> amplitude_ >> timeOffset_;
}

void ModulatorFloat::modulatorChanged_()
{
    if (modulator() == 0)
        sourceType_ = ST_NONE;
    else
    if (qobject_cast<TrackFloat*>(modulator()))
        sourceType_ = ST_TRACK_FLOAT;
    else
    if (qobject_cast<SequenceFloat*>(modulator()))
        sourceType_ = ST_SEQUENCE_FLOAT;
    else
    {
        sourceType_ = ST_NONE;
        MO_LOGIC_ERROR("illegal assignment of modulator '" << modulator()->idName()
                       << "' to ModulatorFloat");
    }
}

Double ModulatorFloat::value(Double time, uint thread) const
{
    if (!modulator() || !modulator()->active(time, thread))
        return 0.0;

    switch (sourceType_)
    {
        case ST_TRACK_FLOAT:
            return amplitude_ *
                    static_cast<TrackFloat*>(modulator())->value(time + timeOffset_, thread);

        case ST_SEQUENCE_FLOAT:
            return amplitude_ *
                    static_cast<SequenceFloat*>(modulator())->value(time + timeOffset_, thread);

        case ST_NONE:
            return 0.0;
    }

    return 0.0;
}


} // namespace MO
