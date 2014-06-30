/** @file axisrotation.cpp

    @brief Rotation about arbitrary axis

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "axisrotation.h"
#include "parameterfloat.h"

namespace MO {

MO_REGISTER_OBJECT(AxisRotation)

AxisRotation::AxisRotation(QObject *parent) :
    Transformation(parent)
{
    setName("AxisRotation");
}

void AxisRotation::createParameters()
{
    angle_ = createFloatParameter("rotx", "angle", 0);
    x_ = createFloatParameter("axis_x", "axis x", 1);
    y_ = createFloatParameter("axis_y", "axis y", 0);
    z_ = createFloatParameter("axis_z", "axis z", 0);
}

void AxisRotation::applyTransformation(Mat4 &matrix, Double time) const
{
    matrix = glm::rotate(matrix,
                (Mat4::value_type)angle_->value(time),
                 Vec3(x_->value(time), y_->value(time), z_->value(time)));
}


} // namespace MO
