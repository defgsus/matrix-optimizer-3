/** @file sequence.cpp

    @brief Sequence class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequence.h"

namespace MO {

MO_REGISTER_OBJECT(Sequence)

Sequence::Sequence(QObject *parent) :
    Object(parent)
{
    setName("Sequence");
}


} // namespace MO
