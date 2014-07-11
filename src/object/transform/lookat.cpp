/** @file lookat.cpp

    @brief Look-at transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#include "lookat.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(LookAt)

LookAt::LookAt(QObject *parent) :
    Transformation(parent)
{
    setName("LookAt");
}

void LookAt::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("lookat", 1);
}

void LookAt::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("lookat", 1);
}


void LookAt::createParameters()
{
    Transformation::createParameters();

    const QString lookTip = tr("Global position to look at"),
                  upTip = tr("unit vector describing the up-axis (internally normalized)");
    x_ = createFloatParameter("x", "look-at x", lookTip, 0);
    y_ = createFloatParameter("y", "look-at y", lookTip, 0);
    z_ = createFloatParameter("z", "look-at z", lookTip, 0);
    upX_ = createFloatParameter("upx", "up x", upTip, 0);
    upY_ = createFloatParameter("upy", "up y", upTip, 1);
    upZ_ = createFloatParameter("upz", "up z", upTip, 0);
}


void LookAt::applyTransformation(Mat4 &matrix, Double time) const
{
    // extract position
    Vec3 pos = Vec3(matrix[3][0], matrix[3][1], matrix[3][2]);

    // forward vector (look-at minus position)
    Vec3 f = glm::normalize(Vec3(x_->value(time), y_->value(time), z_->value(time)) - pos);
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

