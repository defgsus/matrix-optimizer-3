/** @file mirrortrans.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_MIRRORTRANS_H
#define MOSRC_OBJECT_TRANSFORM_MIRRORTRANS_H

#include "Transformation.h"

namespace MO {


class MirrorTrans : public Transformation
{
public:
    MO_OBJECT_CONSTRUCTOR(MirrorTrans);

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, const RenderTime& time) const
                                                        Q_DECL_OVERRIDE;

protected:

    ParameterFloat
        * x_, * y_, * z_,
        * ax_, * ay_, * az_;
    ParameterSelect
        * mPos_, * mOri_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_MIRRORTRANS_H
