/** @file scale.cpp

    @brief scale transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#include "scale.h"
#include "parameterfloat.h"
#include "io/datastream.h"


namespace MO {

MO_REGISTER_OBJECT(Scale)

Scale::Scale(QObject *parent) :
    Transformation(parent)
{
    setName("Scale");
}

void Scale::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("scale", 1);
}

void Scale::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("scale", 1);
}


void Scale::createParameters()
{
    all_ = createFloatParameter("scale_all", "scale", 1);
    x_ = createFloatParameter("scale_x", "x", 1);
    y_ = createFloatParameter("scale_y", "y", 1);
    z_ = createFloatParameter("scale_z", "z", 1);

    //useXYZ_ = createSetting("usexyz", "individual scale", false);
}
/*
void Scale::settingChanged(Setting * s)
{
    if (s == useXYZ_)
    {
        bool xyz = useXYZ_->value();
        x_->setEnabled(xyz);
        y_->setEnabled(xyz);
        z_->setEnabled(xyz);
    }
}
*/
void Scale::applyTransformation(Mat4 &matrix, Double time) const
{
    const float all = all_->value(time);
    matrix = glm::scale(matrix,
                 Vec3(x_->value(time) * all, y_->value(time) * all, z_->value(time) * all));
}


} // namespace MO
