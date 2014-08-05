/** @file modulatorfloat.cpp

    @brief Float modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "modulatorfloat.h"

namespace MO {


ModulatorFloat::ModulatorFloat(const QString &modulatorId, Object *parent)
    : Modulator(modulatorId, parent)
{
}

void ModulatorFloat::serialize(IO::DataStream &) const
{

}

void ModulatorFloat::deserialize(IO::DataStream &)
{

}


} // namespace MO
