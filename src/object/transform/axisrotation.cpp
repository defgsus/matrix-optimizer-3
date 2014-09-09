/** @file axisrotation.cpp

    @brief Rotation about arbitrary axis

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "axisrotation.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"
#include "math/vector.h"


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
    Transformation::createParameters();

    beginParameterGroup("trans", tr("transformation"));

    const QString axisTip = tr("Unit vector describing the axis to rotate around (internally normalized)");
    angle_ = createFloatParameter("a", "angle",
                                  tr("The rotation in degree"), 0);
    x_ = createFloatParameter("x", "axis x", axisTip, 1);
    y_ = createFloatParameter("y", "axis y", axisTip, 0);
    z_ = createFloatParameter("z", "axis z", axisTip, 0);

    endParameterGroup();
}

void AxisRotation::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    matrix = MATH::rotate(matrix,
                (Mat4::value_type)angle_->value(time, thread),
                 Vec3(x_->value(time, thread),
                      y_->value(time, thread),
                      z_->value(time, thread)));
}


} // namespace MO
