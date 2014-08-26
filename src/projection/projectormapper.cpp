/** @file

    @brief Projection mapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "projectormapper.h"
#include "domesettings.h"
#include "math/intersection.h"
#include "io/log.h"

namespace MO {


ProjectorMapper::ProjectorMapper()
    :   valid_      (false),
        aspect_     (0)
{
    recalc_();
}

void ProjectorMapper::setSettings(const ProjectorSettings & s)
{
    set_ = s;
    recalc_();
}


void ProjectorMapper::recalc_()
{
    valid_ = false;

    if (set_.width() <= 0 || set_.height() <= 0)
        return;

    // aspect ratio
    aspect_ = (Float)set_.height()/set_.width();

    // -- calc transformation matrix --

    trans_ = Mat4(1);
    trans_ = glm::rotate(trans_, set_.latitude(), Vec3(0,1,0));
    trans_ = glm::rotate(trans_, -set_.longitude(), Vec3(1,0,0));
    trans_ = glm::translate(trans_, Vec3(0,0,set_.radius()));

    // get actual position
    pos_ = Vec3( trans_ * Vec4(0,0,0,1) );

    // roll-pitch-yaw
    trans_ = glm::rotate(trans_, set_.roll(),      Vec3(0,0,1));
    trans_ = glm::rotate(trans_, set_.yaw(),       Vec3(0,1,0));
    trans_ = glm::rotate(trans_, set_.pitch(),     Vec3(1,0,0));

    valid_ = true;
}

Vec3 ProjectorMapper::getRayOrigin(Float s, Float t) const
{
    s = s * 2 - 1;
    t = (t * 2 - 1) * aspect_;

    // XXX why is s negative??
    Vec3 pos = set_.lensRadius() * std::sqrt((Float)2) * Vec3(-s, t, 0);
    return Vec3(trans_ * Vec4(pos, (Float)1));
}

void ProjectorMapper::getRay(Float s, Float t, Vec3 *ray_origin, Vec3 *ray_direction) const
{
    s = s * 2 - 1;
    t = (t * 2 - 1) * aspect_;

    // XXX why is s negative??
    Vec3 pos = set_.lensRadius() * std::sqrt((Float)2) * Vec3(-s, t, 0);

    Vec3 dir = glm::rotateX(Vec3(0,0,-1), t * set_.fov() * (Float)0.5);
         dir = glm::rotateY(dir, s * set_.fov() * (Float)0.5);

    *ray_origin =    Vec3(trans_ * Vec4(pos, (Float)1));
    *ray_direction = Vec3(trans_ * Vec4(dir, (Float)0));
}

Vec3 ProjectorMapper::mapToDome(Float s, Float t, const DomeSettings & set) const
{
    Vec3 pos, dir;
    getRay(s, t, &pos, &dir);

    Float depth1, depth2;
    if (!MATH::intersect_ray_sphere(pos, dir, Vec3(0,0,0), set.radius(), &depth1, &depth2))
    {
        return pos + dir * (Float)100;
    }

    return pos + dir * depth1;
}

Vec2 ProjectorMapper::mapToSphere(Float, Float) const
{
    const Vec3 ray_origin = Vec3( trans_ * Vec4(0,0,0,1) );
    const Vec3 ray_dir = Vec3( trans_ * Vec4(0,0,-1,0) );

    Float depth1, depth2;

    if (!MATH::intersect_ray_sphere(ray_origin, ray_dir, Vec3(0,0,0), set_.radius(), &depth1, &depth2))
    {
        return Vec2(0,0);
    }

    return Vec2(0,0);
}


} // namespace MO
