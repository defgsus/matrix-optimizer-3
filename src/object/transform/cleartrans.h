/** @file cleartrans.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_CLEARTRANS_H
#define MOSRC_OBJECT_TRANSFORM_CLEARTRANS_H

#include "transformation.h"

namespace MO {


class ClearTrans : public Transformation
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(ClearTrans);

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, Double time, uint thread) const
                                                        Q_DECL_OVERRIDE;

signals:

public slots:

protected:

    ParameterSelect
        * cScale_, *cRot_, *cPos_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_CLEARTRANS_H
