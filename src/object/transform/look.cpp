/** @file Look.cpp

    @brief Look-axis transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#include "look.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(Look)

Look::Look(QObject *parent) :
    Transformation(parent)
{
    setName("Look");
}

void Look::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("look", 1);
}

void Look::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("look", 1);
}


void Look::createParameters()
{
    x_ = createFloatParameter("x", "look x", 0);
    y_ = createFloatParameter("y", "look y", 0);
    z_ = createFloatParameter("z", "look z", -1);
    upX_ = createFloatParameter("upx", "up x", 0);
    upY_ = createFloatParameter("upy", "up y", 1);
    upZ_ = createFloatParameter("upz", "up z", 0);
}


void Look::applyTransformation(Mat4 &matrix, Double time) const
{
    // forward vector
    Vec3 f = glm::normalize(Vec3(x_->value(time), y_->value(time), z_->value(time)));
    // up vector
    Vec3 u = glm::normalize(Vec3(upX_->value(time), upY_->value(time), upZ_->value(time)));
    // right vector
    Vec3 s = glm::normalize(glm::cross(u, f));
    // rebuild up to avoid distortion
    u = glm::cross(f, s);

    Mat4 lookm(1);

    lookm[0][0] =-s.x;
    lookm[0][1] =-s.y;
    lookm[0][2] =-s.z;
    lookm[1][0] = u.x;
    lookm[1][1] = u.y;
    lookm[1][2] = u.z;
    lookm[2][0] =-f.x;
    lookm[2][1] =-f.y;
    lookm[2][2] =-f.z;

    matrix *= lookm;
}


} // namespace MO

