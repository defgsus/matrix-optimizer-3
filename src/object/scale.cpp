/** @file scale.cpp

    @brief scale transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#include "scale.h"
#include "parameterfloat.h"

namespace MO {

MO_REGISTER_OBJECT(Scale)

Scale::Scale(QObject *parent) :
    Transformation(parent)
{
    setName("Scale");
}

void Scale::createParameters()
{
    x_ = createFloatParameter("pos_x", "x", 1);
    y_ = createFloatParameter("pos_y", "y", 1);
    z_ = createFloatParameter("pos_z", "z", 1);
}

void Scale::applyTransformation(Mat4 &matrix, Double time) const
{
    matrix = glm::scale(matrix,
                 Vec3(x_->value(time), y_->value(time), z_->value(time)));
}


} // namespace MO
