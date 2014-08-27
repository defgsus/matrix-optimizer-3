/** @file projectorsettings.cpp

    @brief Settings of a single projector

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include "projectorsettings.h"
#include "io/xmlstream.h"

namespace MO {

ProjectorSettings::ProjectorSettings()
    :   width_      (1280),
        height_     (768),
        fov_        (60),
        lensRadius_ (0),
        latitude_   (0),
        longitude_  (0),
        distance_   (0),
        pitch_      (0),
        yaw_        (0),
        roll_       (0)
{
}

void ProjectorSettings::serialize(IO::XmlStream & io) const
{
    io.newSection("projector");

        io.write("name", name_);
        io.write("width", width_);
        io.write("height", height_);
        io.write("fov", fov_);
        io.write("lens_radius", lensRadius_);
        io.write("latitude", latitude_);
        io.write("longitude", longitude_);
        io.write("distance", distance_);
        io.write("pitch", pitch_);
        io.write("yaw", yaw_);
        io.write("roll", roll_);

    io.endSection();
}

void ProjectorSettings::deserialize(IO::XmlStream & io)
{
    io.verifySection("projector");

        name_ = io.expectString("name");
        width_ = io.expectInt("width");
        height_ = io.expectInt("height");
        fov_ = io.expectFloat("fov");
        lensRadius_ = io.expectFloat("lens_radius");
        latitude_ = io.expectFloat("latitude");
        longitude_ = io.expectFloat("longitude");
        distance_ = io.expectFloat("distance");
        pitch_ = io.expectFloat("pitch");
        yaw_ = io.expectFloat("yaw");
        roll_ = io.expectFloat("roll");
}


} // namespace MO
