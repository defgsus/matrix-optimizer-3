/** @file mix.h

    @brief mixer for contained transformations

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/11/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_MIX_H
#define MOSRC_OBJECT_TRANSFORM_MIX_H

#include "transformation.h"

namespace MO {

class Mix : public Transformation
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Mix);

    virtual Type type() const { return T_TRANSFORMATION_MIX; }

    virtual void createParameters();

    virtual void applyTransformation(Mat4& matrix, Double time) const;

    virtual void childrenChanged();

signals:

public slots:

protected:

    ParameterFloat
        * m_;

    QList<Transformation*> transformations_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_MIX_H
