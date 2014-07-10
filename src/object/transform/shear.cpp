/** @file shear.cpp

    @brief Shear transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#include "shear.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(Shear)

Shear::Shear(QObject *parent) :
    Transformation(parent)
{
    setName("Shear");
}

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
    xy_ = createFloatParameter("xy", "x -> y", 0);
    xz_ = createFloatParameter("xz", "x -> z", 0);
    yx_ = createFloatParameter("yx", "y -> x", 0);
    yz_ = createFloatParameter("yz", "y -> z", 0);
    zx_ = createFloatParameter("zx", "z -> x", 0);
    zy_ = createFloatParameter("zy", "z -> y", 0);
}

void Shear::applyTransformation(Mat4 &matrix, Double time) const
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
