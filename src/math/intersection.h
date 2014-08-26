/** @file intersection.h

    @brief Geometric intersection functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#ifndef MOSRC_MATH_INTERSECTION_H
#define MOSRC_MATH_INTERSECTION_H

#include "types/vector.h"

namespace MO {
namespace MATH {

bool intersect_ray_sphere(const Vec3& ray_origin,
                          const Vec3& ray_direction,
                          const Vec3& sphere_center,
                          Float sphere_radius,
                          Float * depth1,
                          Float * depth2);

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_INTERSECTION_H
