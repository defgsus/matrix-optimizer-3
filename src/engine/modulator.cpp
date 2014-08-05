/** @file modulator.cpp

    @brief Abstract modulator class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "modulator.h"

namespace MO {


Modulator::Modulator(const QString &id, Object *parent)
    : parent_       (parent),
      modulatorId_  (id)
{
}


} // namespace MO
