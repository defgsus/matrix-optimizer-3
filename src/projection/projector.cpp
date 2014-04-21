/***************************************************************************

Copyright (C) 2014  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

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
    aspect_ = (Float)height_/width_;

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
