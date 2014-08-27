/** @file domesettings.cpp

    @brief Dome settings container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include "domesettings.h"
#include "io/xmlstream.h"

namespace MO {


DomeSettings::DomeSettings()
    : radius_       (10),
      coverage_     (180),
      tiltX_        (0),
      tiltZ_        (0)
{
}

void DomeSettings::serialize(IO::XmlStream & io) const
{
    io.newSection("dome");

        io.write("name", name_);
        io.write("radius", radius_);
        io.write("coverage", coverage_);
        io.write("tiltX", tiltX_);
        io.write("tiltZ", tiltZ_);

    io.endSection();
}

void DomeSettings::deserialize(IO::XmlStream & io)
{
    io.verifySection("dome");

        name_ = io.expectString("name");
        radius_ = io.expectFloat("radius");
        coverage_ = io.expectFloat("coverage");
        tiltX_ = io.expectFloat("tiltX");
        tiltZ_ = io.expectFloat("tiltZ");
}


} // namespace MO
