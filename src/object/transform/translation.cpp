/** @file translation.cpp

    @brief object translation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "translation.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"


namespace MO {

MO_REGISTER_OBJECT(Translation)

Translation::Translation(QObject *parent) :
    Transformation(parent)
{
    setName("Translation");
}

void Translation::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("trans", 1);
}

void Translation::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("trans", 1);
}

void Translation::createParameters()
{
    Transformation::createParameters();

    x_ = createFloatParameter("x", tr("x"), tr("Offset on the x axis"), 0.0, 0.2);
    y_ = createFloatParameter("y", tr("y"), tr("Offset on the y axis"), 0.0, 0.2);
    z_ = createFloatParameter("z", tr("z"), tr("Offset on the z axis"), 0.0, 0.2);
}

void Translation::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    matrix = glm::translate(matrix,
                 Vec3(x_->value(time,thread), y_->value(time,thread), z_->value(time,thread)));
}


} // namespace MO
