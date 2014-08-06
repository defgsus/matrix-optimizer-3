/** @file modulator.cpp

    @brief Abstract modulator class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "modulator.h"
#include "io/datastream.h"

namespace MO {


Modulator::Modulator(const QString &id, Object *parent)
    : parent_       (parent),
      modulatorId_  (id)
{
}


void Modulator::serialize(IO::DataStream & io) const
{
    io.writeHeader("mod", 1);

    io << modulatorId_;
}

void Modulator::deserialize(IO::DataStream & io)
{
    io.readHeader("mod", 1);

    io >> modulatorId_;
}


} // namespace MO
