/** @file sequences.cpp

    @brief Sequence container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequences.h"
#include "io/datastream.h"


namespace MO {

MO_REGISTER_OBJECT(Sequences)

Sequences::Sequences(QObject *parent) :
    Object(parent)
{
    setName("SequenceGroup");
}


void Sequences::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("seqs", 1);
}

void Sequences::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("seqs", 1);
}


}

