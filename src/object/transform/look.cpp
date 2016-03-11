/** @file look.cpp

    @brief Look-axis transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#include "look.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "io/datastream.h"
#include "math/vector.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(Look)

Look::Look()
    : Transformation()
{
    setName("Look");
}

Look::~Look() { }

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

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        const QString lookTip = tr("unit vector describing the look axis (internally normalized)"),
                      upTip = tr("unit vector describing the up-axis (internally normalized)");
        x_ = params()->createFloatParameter("x", "look x", lookTip, 0);
        y_ = params()->createFloatParameter("y", "look y", lookTip, 0);
        z_ = params()->createFloatParameter("z", "look z", lookTip, -1);
        upX_ = params()->createFloatParameter("upx", "up x", upTip, 0);
        upY_ = params()->createFloatParameter("upy", "up y", upTip, 1);
        upZ_ = params()->createFloatParameter("upz", "up z", upTip, 0);

    params()->endParameterGroup();
}


void Look::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    // forward vector
    Vec3 f = MATH::normalize_safe(Vec3(x_->value(time),
                                       y_->value(time),
                                       z_->value(time)));
    // up vector
    Vec3 u = MATH::normalize_safe(Vec3(upX_->value(time),
                                       upY_->value(time),
                                       upZ_->value(time)));
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

