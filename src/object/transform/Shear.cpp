/** @file shear.cpp

    @brief Shear transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#include "Shear.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "io/DataStream.h"

namespace MO {

MO_REGISTER_OBJECT(Shear)

Shear::Shear()
    : Transformation()
{
    setName("Shear");
}

Shear::~Shear() { }

void Shear::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("shear", 1);
}

void Shear::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("shear", 1);
}


void Shear::createParameters()
{
    Transformation::createParameters();

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        xy_ = params()->createFloatParameter("xy", "x -> y", tr("Shear influence of x axis on y axis"), 0);
        xz_ = params()->createFloatParameter("xz", "x -> z", tr("Shear influence of x axis on z axis"), 0);
        yx_ = params()->createFloatParameter("yx", "y -> x", tr("Shear influence of y axis on x axis"), 0);
        yz_ = params()->createFloatParameter("yz", "y -> z", tr("Shear influence of y axis on z axis"), 0);
        zx_ = params()->createFloatParameter("zx", "z -> x", tr("Shear influence of z axis on x axis"), 0);
        zy_ = params()->createFloatParameter("zy", "z -> y", tr("Shear influence of z axis on y axis"), 0);

    params()->endParameterGroup();
}

void Shear::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    const Double
        xy = xy_->value(time),
        xz = xz_->value(time),
        yx = yx_->value(time),
        yz = yz_->value(time),
        zx = zx_->value(time),
        zy = zy_->value(time);

    Mat4 shear(1);
    shear[0][1] = xy;
    shear[0][2] = xz;
    shear[1][0] = yx;
    shear[1][2] = yz;
    shear[2][0] = zx;
    shear[2][1] = zy;

    matrix *= shear;
}


} // namespace MO
