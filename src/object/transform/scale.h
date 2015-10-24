/** @file scale.h

    @brief scale transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_SCALE_H
#define MOSRC_OBJECT_TRANSFORM_SCALE_H

#include "transformation.h"

namespace MO {


class Scale : public Transformation
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Scale);

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, const RenderTime& time) const
                                                            Q_DECL_OVERRIDE;

signals:

public slots:

protected:

    ParameterFloat * all_, * x_, * y_, * z_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_SCALE_H
