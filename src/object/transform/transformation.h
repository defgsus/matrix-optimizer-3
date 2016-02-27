/** @file transformation.h

    @brief abstract object transformation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_TRANSFORMATION_H
#define MOSRC_OBJECT_TRANSFORM_TRANSFORMATION_H

#include "object/object.h"

namespace MO {


class Transformation : public Object
{
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(Transformation)

    virtual Type type() const { return T_TRANSFORMATION; }
    virtual bool isTransformation() const { return true; }

    virtual void applyTransformation(Mat4& matrix, const RenderTime& time) const = 0;

};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_TRANSFORMATION_H
