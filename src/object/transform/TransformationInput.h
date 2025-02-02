/** @file transformationinput.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_TRANSFORMATIONINPUT_H
#define MOSRC_OBJECT_TRANSFORM_TRANSFORMATIONINPUT_H

#include "Transformation.h"

namespace MO {

/** A Transformation class that pulls-in another transformation
    through a parameter */
class TransformationInput : public Transformation
{
public:
    MO_OBJECT_CONSTRUCTOR(TransformationInput);

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, const RenderTime& time) const
                                                            Q_DECL_OVERRIDE;
protected:

    ParameterTransformation * p_trans_;
    ParameterSelect * p_apply_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_TRANSFORMATIONINPUT_H
