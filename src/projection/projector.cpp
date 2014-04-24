/** @file

    @brief Projector/projection settings

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "projector.h"

namespace MO {


Projector::Projector()
    :   width_      (1280),
        height_     (768),
        fov_        (60),
        latitude_   (0),
        longitude_  (0),
        radius_     (10),
        pitch_      (0),
        yaw_        (0),
        roll_       (0),

        valid_      (false),
        aspect_     (0)
{
    recalc_();
}


void Projector::recalc_()
{
    valid_ = false;

    if (width_ <= 0 || height_ <= 0)
        return;

    // aspect ratio
    aspect_ = (Float)width_/height_;

    // get actual position
    pos_ = glm::rotateY(glm::rotateX(Vec3(0,0,-radius_), longitude_), latitude_);

    // roll-pitch-yaw matrix
    rpy_ = Mat4(1);
    rpy_ = glm::rotate(rpy_, roll_,      Vec3(0,0,1));
    rpy_ = glm::rotate(rpy_, yaw_,       Vec3(0,1,0));
    rpy_ = glm::rotate(rpy_, pitch_,     Vec3(1,0,0));
    rpy_ = glm::rotate(rpy_, longitude_, Vec3(-1,0,0));
    rpy_ = glm::rotate(rpy_, latitude_,  Vec3(0,-1,0));

    valid_ = true;
}


} // namespace MO
