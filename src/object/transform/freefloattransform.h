/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/21/2015</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_FREEFLOATTRANSFORM_H
#define MOSRC_OBJECT_TRANSFORM_FREEFLOATTRANSFORM_H

#include "transformation.h"

namespace MO {

class FreeFloatCamera;

class FreeFloatTransform : public Transformation
{
public:
    MO_OBJECT_CONSTRUCTOR(FreeFloatTransform);

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, const RenderTime& time) const
                                                        Q_DECL_OVERRIDE;
protected:

    FreeFloatCamera * freefloat_;
    ParameterFloat
        * acc_x_, * acc_y_, * acc_z_,
        * rx_, * ry_, * rz_,
        * vel_, *velr_, *damp_;
    ParameterCallback * reset_;
    mutable bool doReset_;
};


} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_FREEFLOATTRANSFORM_H
