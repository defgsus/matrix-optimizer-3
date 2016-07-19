/** @file mix.h

    @brief mixer for contained transformations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/11/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_MIX_H
#define MOSRC_OBJECT_TRANSFORM_MIX_H

#include "Transformation.h"

namespace MO {

class Mix : public Transformation
{
public:
    MO_OBJECT_CONSTRUCTOR(Mix);

    virtual Type type() const { return T_TRANSFORMATION_MIX; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void applyTransformation(Mat4& matrix, const RenderTime& time) const
                                        Q_DECL_OVERRIDE;

    virtual void childrenChanged() Q_DECL_OVERRIDE;

protected:

    ParameterFloat
        * m_;

    QList<Transformation*> transformations_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_MIX_H
