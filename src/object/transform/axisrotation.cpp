/** @file axisrotation.cpp

    @brief Rotation about arbitrary axis

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "axisrotation.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"


namespace MO {

MO_REGISTER_OBJECT(AxisRotation)

AxisRotation::AxisRotation(QObject *parent) :
    Transformation(parent)
{
    setName("Rotation");
}

void AxisRotation::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("arot", 1);
}

void AxisRotation::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("arot", 1);
}


void AxisRotation::createParameters()
{
    angle_ = createFloatParameter("a", "angle", 0);
    x_ = createFloatParameter("x", "axis x", 1);
    y_ = createFloatParameter("y", "axis y", 0);
    z_ = createFloatParameter("z", "axis z", 0);
}

void AxisRotation::applyTransformation(Mat4 &matrix, Double time) const
{
    matrix = glm::rotate(matrix,
                (Mat4::value_type)angle_->value(time),
                 Vec3(x_->value(time), y_->value(time), z_->value(time)));
}


} // namespace MO
