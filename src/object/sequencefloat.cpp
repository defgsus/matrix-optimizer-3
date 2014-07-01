/** @file sequencefloat.cpp

    @brief Float sequence

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequencefloat.h"

namespace MO {

MO_REGISTER_OBJECT(SequenceFloat)

SequenceFloat::SequenceFloat(QObject *parent)
    :   Sequence(parent)
{
}


} // namespace MO
