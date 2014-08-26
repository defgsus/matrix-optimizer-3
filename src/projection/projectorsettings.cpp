/** @file projectorsettings.cpp

    @brief Settings of a single projector

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include "projectorsettings.h"

namespace MO {

ProjectorSettings::ProjectorSettings()
    :   width_      (1280),
        height_     (768),
        fov_        (60),
        lensRadius_ (0),
        latitude_   (0),
        longitude_  (0),
        radius_     (10),
        pitch_      (0),
        yaw_        (0),
        roll_       (0)
{
}

} // namespace MO
