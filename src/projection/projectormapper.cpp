/** @file

    @brief Projection mapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "projector.h"
#include "math/intersection.h"

namespace MO {


ProjectorMapper::ProjectorMapper()
    :   valid_      (false),
        aspect_     (0)
{
    recalc_();
}


void ProjectorMapper::recalc_()
{
    valid_ = false;

    if (set_.width() <= 0 || set_.height() <= 0)
        return;

    // aspect ratio
    aspect_ = (Float)set_.width()/set_.height();

    // get actual position
    pos_ = glm::rotateY(glm::rotateX(Vec3(0,0,-set_.radius()), set_.longitude()), set_.latitude());

    // roll-pitch-yaw matrix
    rpy_ = Mat4(1);
    rpy_ = glm::rotate(rpy_, set_.roll(),      Vec3(0,0,1));
    rpy_ = glm::rotate(rpy_, set_.yaw(),       Vec3(0,1,0));
    rpy_ = glm::rotate(rpy_, set_.pitch(),     Vec3(1,0,0));
    rpy_ = glm::rotate(rpy_, set_.longitude(), Vec3(-1,0,0));
    rpy_ = glm::rotate(rpy_, set_.latitude(),  Vec3(0,-1,0));

    valid_ = true;
}

Vec2 ProjectorMapper::mapToSphere(int pixel_x, int pixel_y) const
{
    const Vec3 ray_origin = pos_;
    const Vec3 ray_dir = Vec3( rpy_ * Vec4(0,0,-1,0) );

    Float depth1, depth2;

    if (!MATH::intersect_ray_sphere(ray_origin, ray_dir, Vec3(0,0,0), set_.radius(), &depth1, &depth2))
    {
        return Vec2(0,0);
    }


}


} // namespace MO
