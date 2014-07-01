/** @file sequences.cpp

    @brief Sequence container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequences.h"

namespace MO {

MO_REGISTER_OBJECT(Sequences)

Sequences::Sequences(QObject *parent) :
    Object(parent)
{
    setName("SequenceGroup");
}


}

