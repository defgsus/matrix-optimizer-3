/** @file scale.cpp

    @brief scale transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#include "scale.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
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
    Transformation::createParameters();

    params()->beginParameterGroup("trans", tr("transformation"));

        all_ = params()->createFloatParameter("all", "scale", tr("Scale/multiplier for the whole"), 1, 0.1);
        x_ = params()->createFloatParameter("x", "x", tr("Additional scale for the x axis"), 1, 0.1);
        y_ = params()->createFloatParameter("y", "y", tr("Additional scale for the y axis"), 1, 0.1);
        z_ = params()->createFloatParameter("z", "z", tr("Additional scale for the z axis"), 1, 0.1);

    params()->endParameterGroup();
}

void Scale::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    const float all = all_->value(time, thread);
    matrix = glm::scale(matrix,
                 Vec3(x_->value(time, thread) * all,
                      y_->value(time, thread) * all,
                      z_->value(time, thread) * all));
}


} // namespace MO
