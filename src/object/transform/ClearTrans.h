/** @file cleartrans.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_CLEARTRANS_H
#define MOSRC_OBJECT_TRANSFORM_CLEARTRANS_H

#include "Transformation.h"

namespace MO {


class ClearTrans : public Transformation
{
public:
    MO_OBJECT_CONSTRUCTOR(ClearTrans);

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, const RenderTime& time) const
                                                        Q_DECL_OVERRIDE;

protected:

    ParameterSelect
        * cScale_, *cRot_, *cPos_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_CLEARTRANS_H
