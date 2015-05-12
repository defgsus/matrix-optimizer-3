/** @file transformationinput.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_TRANSFORMATIONINPUT_H
#define MOSRC_OBJECT_TRANSFORM_TRANSFORMATIONINPUT_H

#include "transformation.h"

namespace MO {

/** A Transformation class that pulls-in another transformation
    through a parameter */
class TransformationInput : public Transformation
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(TransformationInput);

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, Double time, uint thread) const
                                                            Q_DECL_OVERRIDE;

signals:

public slots:

protected:

    ParameterTransformation * p_trans_;
    ParameterSelect * p_apply_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_TRANSFORMATIONINPUT_H
