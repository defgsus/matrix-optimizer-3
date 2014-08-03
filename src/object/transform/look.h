/** @file lookat.h

    @brief Look-axis transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_LOOK_H
#define MOSRC_OBJECT_TRANSFORM_LOOK_H

#include "transformation.h"

namespace MO {


class Look : public Transformation
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Look);

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, Double time, uint thread) const
                                                        Q_DECL_OVERRIDE;

signals:

public slots:

protected:

    ParameterFloat
        * x_, * y_, * z_,
        * upX_, * upY_, * upZ_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_LOOKAT_H
