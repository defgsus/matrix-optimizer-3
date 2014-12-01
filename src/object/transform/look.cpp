/** @file Look.cpp

    @brief Look-axis transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#include "look.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"
#include "math/vector.h"
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
    Transformation::createParameters();

    beginParameterGroup("trans", tr("transformation"));

    const QString lookTip = tr("unit vector describing the look axis (internally normalized)"),
                  upTip = tr("unit vector describing the up-axis (internally normalized)");
    x_ = createFloatParameter("x", "look x", lookTip, 0);
    y_ = createFloatParameter("y", "look y", lookTip, 0);
    z_ = createFloatParameter("z", "look z", lookTip, -1);
    upX_ = createFloatParameter("upx", "up x", upTip, 0);
    upY_ = createFloatParameter("upy", "up y", upTip, 1);
    upZ_ = createFloatParameter("upz", "up z", upTip, 0);

    endParameterGroup();
}


void Look::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    // forward vector
    Vec3 f = MATH::normalize_safe(Vec3(x_->value(time, thread),
                                 y_->value(time, thread),
                                 z_->value(time, thread)));
    // up vector
    Vec3 u = MATH::normalize_safe(Vec3(upX_->value(time, thread),
                                 upY_->value(time, thread),
                                 upZ_->value(time, thread)));
    // right vector
    Vec3 s = MATH::normalize_safe(glm::cross(f, u));
    // rebuild up to avoid distortion
    u = glm::cross(s, f);

    Mat4 lookm(1);

    lookm[0][0] = s.x;
    lookm[0][1] = s.y;
    lookm[0][2] = s.z;
    lookm[1][0] = u.x;
    lookm[1][1] = u.y;
    lookm[1][2] = u.z;
    lookm[2][0] =-f.x;
    lookm[2][1] =-f.y;
    lookm[2][2] =-f.z;

    matrix *= lookm;
}


} // namespace MO

