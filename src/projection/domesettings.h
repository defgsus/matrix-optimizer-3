/** @file domesettings.h

    @brief Dome settings container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#ifndef MOSRC_PROJECTION_DOMESETTINGS_H
#define MOSRC_PROJECTION_DOMESETTINGS_H

#include "types/float.h"

namespace MO {


class DomeSettings
{
public:
    DomeSettings();

    // --------- getter -------------

    /** Radius in meters */
    Float radius() const { return radius_; }

    /** Dome coverage in degree, e.g. 180 == half-sphere */
    Float coverage() const { return coverage_; }

    /** Tilt on x-axis in degree */
    Float tiltX() const { return tiltX_; }

    /** Tilt on z-axis in degree */
    Float tiltZ() const { return tiltZ_; }

    // --------- setter -------------

    void setRadius(Float r) { radius_ = r; }
    void setCoverage(Float c) { coverage_ = c; }
    void setTiltX(Float t) { tiltX_ = t; }
    void setTiltZ(Float t) { tiltZ_ = t; }

private:

    Float radius_, coverage_, tiltX_, tiltZ_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_DOMESETTINGS_H
