/** @file axisrotation.h

    @brief Rotation about arbitrary axis

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_AXISROTATION_H
#define MOSRC_OBJECT_TRANSFORM_AXISROTATION_H


#include "Transformation.h"

namespace MO {


class AxisRotation : public Transformation
{
public:
    MO_OBJECT_CONSTRUCTOR(AxisRotation);

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void applyTransformation(
            Mat4& matrix, const RenderTime& time) const Q_DECL_OVERRIDE;

protected:

    ParameterFloat * angle_, * x_, * y_, * z_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_AXISROTATION_H
