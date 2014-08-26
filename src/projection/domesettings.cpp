/** @file domesettings.cpp

    @brief Dome settings container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include "domesettings.h"

namespace MO {


DomeSettings::DomeSettings()
    : radius_       (10),
      coverage_     (180),
      tiltX_        (0),
      tiltZ_        (0)
{
}


} // namespace MO
