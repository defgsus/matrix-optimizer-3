/** @file intersection.h

    @brief Geometric intersection functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#ifndef MOSRC_MATH_INTERSECTION_H
#define MOSRC_MATH_INTERSECTION_H

#include <QVector>

#include "types/vector.h"

namespace MO {
namespace MATH {

/** Returns true if both components of @p v are in the range [@p mi,@p ma].
    In other words, if the point @p v, is within the square <mi,mi> - <ma,ma> */
bool inside_range(const Vec2& v, Float mi, Float ma);

/** Returns true if both components of @p v are in the range [@p mi,@p ma].
    In other words, if the point @p v, is within the mi - ma */
bool inside_range(const Vec2 &v, const Vec2& mi, const Vec2& ma);

/** Returns true if lines A and B intersect.
    Returns the intersection parameter in @p t [0,1], if @p t != NULL.
    Actual intersection point can be found with: lineA1 + t * (lineA2 - lineA1)
    @note Currently does not check for parallel lines intersections
    */
bool intersect_line_line(const Vec2& lineA1,
                         const Vec2& lineA2,
                         const Vec2& lineB1,
                         const Vec2& lineB2,
                         Float * t = 0);

bool intersect_ray_sphere(const Vec3& ray_origin,
                          const Vec3& ray_direction,
                          const Vec3& sphere_center,
                          Float sphere_radius,
                          Float * depth1 = 0,
                          Float * depth2 = 0);





} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_INTERSECTION_H
