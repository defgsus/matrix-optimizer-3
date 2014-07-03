/** @file transformation.h

    @brief abstract object transformation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "object.h"

namespace MO {


class Transformation : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(Transformation)

    virtual Type type() const { return T_TRANSFORMATION; }
    virtual bool isTransformation() const { return true; }

    virtual void applyTransformation(Mat4& matrix, Double time) const = 0;

signals:

public slots:

};

} // namespace MO

#endif // TRANSFORMATION_H
