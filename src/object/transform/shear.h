/** @file shear.h

    @brief Shear transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_SHEAR_H
#define MOSRC_OBJECT_TRANSFORM_SHEAR_H

#include "transformation.h"

namespace MO {

class Shear : public Transformation
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Shear);

    virtual void createParameters();

    virtual void applyTransformation(Mat4& matrix, Double time) const;

signals:

public slots:

protected:

    ParameterFloat
        *xy_, *xz_, *yx_, *yz_, *zx_, *zy_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_SHEAR_H
