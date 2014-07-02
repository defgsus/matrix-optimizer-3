/** @file sequence.cpp

    @brief Abstract sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequence.h"
#include "io/datastream.h"

namespace MO {

const Double Sequence::minimumSpeed = 0.001;

Sequence::Sequence(QObject *parent) :
    Object      (parent),
    start_      (0.0),
    length_     (60.0),
    loopStart_  (0.0),
    loopLength_ (length_),
    speed_      (1.0),
    looping_    (false)
{
    setName("Sequence");
}

void Sequence::serialize(IO::DataStream &io) const
{
    io.writeHeader("seq", 1);

    io << start_ << length_ << loopStart_ << loopLength_ << speed_ << (qint8)looping_;
}

void Sequence::deserialize(IO::DataStream &io)
{
    io.readHeader("seq", 1);

    qint8 looping;
    io >> start_ >> length_ >> loopStart_ >> loopLength_ >> speed_ >> looping;
    looping_ = looping;
}

} // namespace MO
