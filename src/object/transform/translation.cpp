/** @file translation.cpp

    @brief object translation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "translation.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"


namespace MO {

MO_REGISTER_OBJECT(Translation)

Translation::Translation()
    : Transformation()
{
    setName("Translation");
}

Translation::~Translation() { }


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

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        x_ = params()->createFloatParameter("x", tr("x"), tr("Offset on the x axis"), 0.0, 0.1);
        y_ = params()->createFloatParameter("y", tr("y"), tr("Offset on the y axis"), 0.0, 0.1);
        z_ = params()->createFloatParameter("z", tr("z"), tr("Offset on the z axis"), 0.0, 0.1);

    params()->endParameterGroup();
}

void Translation::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    matrix = glm::translate(matrix,
                 Vec3(x_->value(time), y_->value(time), z_->value(time)));
}


} // namespace MO
