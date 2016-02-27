/** @file axisrotation.cpp

    @brief Rotation about arbitrary axis

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "axisrotation.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"
#include "math/vector.h"


namespace MO {

MO_REGISTER_OBJECT(AxisRotation)

AxisRotation::AxisRotation()
    : Transformation()
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

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        const QString axisTip = tr("Unit vector describing the axis to rotate around (internally normalized)");
        angle_ = params()->createFloatParameter("a", "angle",
                                      tr("The rotation in degree"), 0);
        x_ = params()->createFloatParameter("x", "axis x", axisTip, 1);
        y_ = params()->createFloatParameter("y", "axis y", axisTip, 0);
        z_ = params()->createFloatParameter("z", "axis z", axisTip, 0);

    params()->endParameterGroup();
}

void AxisRotation::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    matrix = MATH::rotate(matrix,
                (Mat4::value_type)angle_->value(time),
                 Vec3(x_->value(time),
                      y_->value(time),
                      z_->value(time)));
}


} // namespace MO
