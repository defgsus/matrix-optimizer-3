/** @file translation.cpp

    @brief object translation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "translation.h"
#include "parameterfloat.h"
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
    x_ = createFloatParameter("pos_x", "x", 0);
    y_ = createFloatParameter("pos_y", "y", 0);
    z_ = createFloatParameter("pos_z", "z", 0);
}

void Translation::applyTransformation(Mat4 &matrix, Double time) const
{
    matrix = glm::translate(matrix,
                 Vec3(x_->value(time), y_->value(time), z_->value(time)));
}


} // namespace MO
