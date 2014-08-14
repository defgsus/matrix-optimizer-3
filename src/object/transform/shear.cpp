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
    Transformation::createParameters();

    beginParameterGroup("trans", tr("transformation"));

    xy_ = createFloatParameter("xy", "x -> y", tr("Shear influence of x axis on y axis"), 0);
    xz_ = createFloatParameter("xz", "x -> z", tr("Shear influence of x axis on z axis"), 0);
    yx_ = createFloatParameter("yx", "y -> x", tr("Shear influence of y axis on x axis"), 0);
    yz_ = createFloatParameter("yz", "y -> z", tr("Shear influence of y axis on z axis"), 0);
    zx_ = createFloatParameter("zx", "z -> x", tr("Shear influence of z axis on x axis"), 0);
    zy_ = createFloatParameter("zy", "z -> y", tr("Shear influence of z axis on y axis"), 0);

    endParameterGroup();
}

void Shear::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    const Double
        xy = xy_->value(time, thread),
        xz = xz_->value(time, thread),
        yx = yx_->value(time, thread),
        yz = yz_->value(time, thread),
        zx = zx_->value(time, thread),
        zy = zy_->value(time, thread);

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
