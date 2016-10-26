/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_VALUEFLOATMATRIXINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_VALUEFLOATMATRIXINTERFACE_H

#include "types/time.h"
#include "math/FloatMatrix.h"

namespace MO {

/** A float matrix value output. */
class ValueFloatMatrixInterface
{
public:
    virtual ~ValueFloatMatrixInterface() { }

    virtual FloatMatrix valueFloatMatrix(
                uint channel, const RenderTime& time) const = 0;

    virtual bool hasFloatMatrixChanged(
            uint channel, const RenderTime& time) const = 0;
};

} // namespace MO


#endif // MOSRC_OBJECT_INTERFACE_VALUEFLOATMATRIXINTERFACE_H

