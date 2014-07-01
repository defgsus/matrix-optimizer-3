/** @file sequence.cpp

    @brief Abstract sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequence.h"
#include "io/datastream.h"

namespace MO {

Sequence::Sequence(QObject *parent) :
    Object      (parent),
    startTime_  (0.0),
    length_     (60.0),
    loopStart_  (0.0),
    loopLength_ (length_)
{
    setName("Sequence");
}

void Sequence::serialize(IO::DataStream &io) const
{
    io.writeHeader("seq", 1);

    io << startTime_ << length_ << loopStart_ << loopLength_;
}

void Sequence::deserialize(IO::DataStream &io)
{
    io.readHeader("seq", 1);

    io >> startTime_ >> length_ >> loopStart_ >> loopLength_;
}

} // namespace MO
