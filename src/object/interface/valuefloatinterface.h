/** @file valuefloatinterface.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.04.2015</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_VALUEFLOATINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_VALUEFLOATINTERFACE_H

#include "types/time.h"

namespace MO {

/** A float value output.
    *Currently* single channel... */
class ValueFloatInterface
{
public:
    virtual ~ValueFloatInterface() { }

    virtual Double valueFloat(uint channel, const RenderTime& time) const = 0;

};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_VALUEFLOATINTERFACE_H
