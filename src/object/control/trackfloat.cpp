/** @file trackfloat.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/6/2014</p>
*/

#include "trackfloat.h"
#include "io/datastream.h"
#include "io/log.h"
#include "sequencefloat.h"

namespace MO {

MO_REGISTER_OBJECT(TrackFloat)

TrackFloat::TrackFloat(QObject *parent) :
    Track(parent)
{
    setName("TrackFloat");
}

void TrackFloat::serialize(IO::DataStream & io) const
{
    Track::serialize(io);
    io.writeHeader("trkf", 1);
}

void TrackFloat::deserialize(IO::DataStream & io)
{
    Track::deserialize(io);
    io.readHeader("trkf", 1);
}

Double TrackFloat::value(Double time, uint thread) const
{
    Double v = 0.0;
    for (auto s : sequences_)
    {
        if (time >= s->start() && time <= s->end()
                && s->active(time, thread))
                    v += s->value(time, thread);
    }

    return v;
}

void TrackFloat::getValues(Double time, uint thread, Double timeIncrement, uint number, Double *ptr) const
{
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(time, thread);
        time += timeIncrement;
    }
}

void TrackFloat::getValues(Double time, uint thread, Double timeIncrement, uint number, F32 *ptr) const
{
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(time, thread);
        time += timeIncrement;
    }
}

void TrackFloat::collectModulators()
{
    MO_DEBUG_MOD("TrackFloat::collectModulators()");

    sequences_ = findChildObjects<SequenceFloat>();
}

QList<Object*> TrackFloat::getModulatingObjects() const
{
    QList<Object*> list(Object::getModulatingObjects());

    for (auto s : sequences_)
        list.append(s);

    for (auto s : sequences_)
        list.append(s->getModulatingObjects());

    return list;
}

} // namespace MO
