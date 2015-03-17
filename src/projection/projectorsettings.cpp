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
    :   id_         (0),
        width_      (1920),
        height_     (1080),
        fov_        (30),
        lensRadius_ (0.01),
        latitude_   (0),
        longitude_  (0),
        distance_   (0),
        pitch_      (0),
        yaw_        (0),
        roll_       (0),
        offsetX_    (0),
        offsetY_    (0),
        fisheyeness_(0)
{
}

void ProjectorSettings::serialize(IO::XmlStream & io) const
{
    io.newSection("projector");

        io.write("version", 4);

        io.write("id", id_);
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

        // v3
        io.write("offset_x", offsetX_);
        io.write("offset_y", offsetY_);

        // v4
        io.write("fisheyeness", fisheyeness_);

    io.endSection();
}

void ProjectorSettings::deserialize(IO::XmlStream & io)
{
    io.verifySection("projector");

        int ver = io.expectInt("version");
        if (ver >= 2)
            id_ = io.expectInt("id");

        name_ = io.expectString("name");
        width_ = io.expectInt("width");
        height_ = io.expectInt("height");
        fov_ = io.expectFloat("fov");
        lensRadius_ = std::max(Float(0.00001), io.expectFloat("lens_radius"));
        latitude_ = io.expectFloat("latitude");
        longitude_ = io.expectFloat("longitude");
        distance_ = io.expectFloat("distance");
        pitch_ = io.expectFloat("pitch");
        yaw_ = io.expectFloat("yaw");
        roll_ = io.expectFloat("roll");

        // v3
        offsetX_ = io.readFloat("offset_x", 0.f);
        offsetY_ = io.readFloat("offset_y", 0.f);

        // v4
        fisheyeness_ = io.readFloat("fisheyeness", 0.0);
}

bool ProjectorSettings::operator == (const ProjectorSettings& o) const
{
    return name_ == o.name_
            && width_ == o.width_
            && height_ == o.height_
            && fov_ == o.fov_
            && lensRadius_ == o.lensRadius_
            && latitude_ == o.latitude_
            && longitude_ == o.longitude_
            && distance_ == o.distance_
            && pitch_ == o.pitch_
            && yaw_ == o.yaw_
            && roll_ == o.roll_
            && offsetX_ == o.offsetX_
            && offsetY_ == o.offsetY_
            && fisheyeness_ == o.fisheyeness_
            //&& overlapAreas_ == o.overlapAreas_
            ;
}

} // namespace MO
