/** @file valuetransformationinterface.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_VALUETRANSFORMATIONINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_VALUETRANSFORMATIONINTERFACE_H

#include "types/vector.h"

namespace MO {

/** A matrix value output. */
class ValueTransformationInterface
{
public:
    virtual ~ValueTransformationInterface() { }

    virtual Mat4 valueTransformation(Double time, uint thread) const = 0;

};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_VALUETRANSFORMATIONINTERFACE_H
