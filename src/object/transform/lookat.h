/** @file lookat.h

    @brief Look-at transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_LOOKAT_H
#define MOSRC_OBJECT_TRANSFORM_LOOKAT_H

#include "transformation.h"

namespace MO {


class LookAt : public Transformation
{
    Q_OBJECT
public:

    enum LookMode
    {
        LM_LOCAL,
        LM_GLOBAL
    };


    MO_OBJECT_CONSTRUCTOR(LookAt);

    virtual void createParameters();

    virtual void applyTransformation(Mat4& matrix, Double time) const;

signals:

public slots:

protected:

    ParameterFloat
        * x_, * y_, * z_,
        * upX_, * upY_, * upZ_;
    ParameterSelect * lookMode_, * upMode_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_LOOKAT_H

