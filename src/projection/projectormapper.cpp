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
#include "math/constants.h"
#include "io/log.h"

namespace MO {


ProjectorMapper::ProjectorMapper()
    :   valid_      (false),
        aspect_     (0)
{
    recalc_();
}

void ProjectorMapper::setSettings(const DomeSettings & ds, const ProjectorSettings & s)
{
    domeSet_ = ds;
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
    trans_ = glm::translate(trans_, Vec3(0,0,domeSet_.radius() + set_.distance()));

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

    Vec3 pos = set_.lensRadius() * std::sqrt((Float)2) * Vec3(s, t, 0);
    return Vec3(trans_ * Vec4(pos, (Float)1));
}

void ProjectorMapper::getRay(Float s, Float t, Vec3 *ray_origin, Vec3 *ray_direction) const
{
    s = s * 2 - 1;
    t = (t * 2 - 1) * aspect_;

    Float lensfac = set_.lensRadius() * std::sqrt((Float)2);
    Vec3 pos = lensfac * Vec3(s, t, 0);


    // size of projection - one unit away
    //    c² = 2a²(1-cos(gamma))
    // -> c = sqrt(2*(1-cos(gamma))
    // and hc = b * sin(alpha)
    // and gamma = 180 - 2*alpha
    // ->  alpha = (180 - gamma) / 2
    const Float gamma = MATH::deg_to_rad(set_.fov());
    const Float c = std::sqrt((Float)2*((Float)1-std::cos(gamma)))
               // XXX lensradius is not calculated properly here
                - (Float)0.135 * lensfac;
    const Float alpha = (Float)0.5 * ((Float)180 - set_.fov());
    const Float hc = std::sin(MATH::deg_to_rad(alpha));
    Vec3 dir = glm::normalize(Vec3(s*c*(Float)0.5, t*c*(Float)0.5, -hc));

    //MO_DEBUG("dir " << dir << ", len " << glm::length(dir));

    *ray_origin =    Vec3(trans_ * Vec4(pos, (Float)1));
    *ray_direction = Vec3(trans_ * Vec4(dir, (Float)0));
}

Vec3 ProjectorMapper::mapToDome(Float s, Float t) const
{
    Vec3 pos, dir;
    getRay(s, t, &pos, &dir);

    Float depth1, depth2;
    if (!MATH::intersect_ray_sphere(pos, dir, Vec3(0,0,0), domeSet_.radius(), &depth1, &depth2))
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

    if (!MATH::intersect_ray_sphere(
                ray_origin, ray_dir, Vec3(0,0,0), domeSet_.radius(), &depth1, &depth2))
    {
        return Vec2(0,0);
    }

    return Vec2(0,0);
}


void ProjectorMapper::findCenterProjection() const
{
    // find corners of projected area
    const Vec3
            bl = mapToDome(0,0),
            br = mapToDome(1,0),
            tl = mapToDome(0,1),
            tr = mapToDome(1,1),
            ml = mapToDome(0,0.5),
            mr = mapToDome(1,0.5);
    /*
    // project back to identity
    const Mat4 itrans = glm::inverse(trans_);
    const Vec3
            iml = Vec3(itrans * Vec4(ml,1)),
            imr = Vec3(itrans * Vec4(mr,1)),
            ibl = Vec3(itrans * Vec4(bl,1)),
            ibr = Vec3(itrans * Vec4(br,1)),
            itl = Vec3(itrans * Vec4(tl,1)),
            itr = Vec3(itrans * Vec4(tr,1));

    MO_DEBUG("iml = " << iml << ", imr = " << imr);
    */

    // horizontal field of view
    const Float adot = glm::dot(glm::normalize(ml), glm::normalize(mr));
    const Float angle = MATH::rad_to_deg(std::acos(adot));
    // aspect ratio
    //const Float aspect =

    MO_DEBUG("angle = " << angle);
}

} // namespace MO
