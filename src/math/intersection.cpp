/** @file intersection.cpp

    @brief Geometric intersection functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include "intersection.h"

namespace MO {
namespace MATH {

namespace { const Float EPSILON = 0.00001; }

bool inside_range(const Vec2 &v, Float mi, Float ma)
{
    return v[0] >= mi && v[1] >= mi && v[0] <= ma && v[1] <= ma;
}


bool intersect_line_line(const Vec2& A1,
                         const Vec2& A2,
                         const Vec2& B1,
                         const Vec2& B2,
                         Float * t)
{
    const Vec2
            u = A2 - A1,
            v = B2 - B1,
            w = A1 - B1;
    const Float D = u[0] * v[1] - u[1] * v[0];

    // parallel?
    if (std::abs(D) < EPSILON)
        return false;

    const Float iA = (v[0] * w[1] - v[1] * w[0]) / D;
    if (iA < 0 || iA > 1)
        return false;

    const Float iB = (u[0] * w[1] - u[1] * w[0]) / D;
    if (iB < 0 || iB > 1)
        return false;

    if (t)
        *t = iA;

    return true;
}



// implementation from povray source :)
bool intersect_ray_sphere(const Vec3& ray_origin,
                          const Vec3& ray_direction,
                          const Vec3& sphere_center,
                          Float sphere_radius,
                          Float * depth1,
                          Float * depth2)
{
    const Vec3 origin_to_center = sphere_center - ray_origin;

    const Float oc_squared = glm::dot(origin_to_center, origin_to_center);

    const Float closest = glm::dot(origin_to_center, ray_direction);

    const Float radius2 = sphere_radius * sphere_radius;

    if (oc_squared >= radius2 && closest < EPSILON)
        return false;

    const Float half_chord2 = radius2 - oc_squared + closest * closest;

    if (half_chord2 > EPSILON)
    {
        const Float half_chord = std::sqrt(half_chord2);

        if (depth1)
            *depth1 = closest + half_chord;
        if (depth2)
            *depth2 = closest - half_chord;

        return true;
    }

    return false;
}




/*
QVector<Vec2> get_intersection(const QVector<Vec2> &poly, const Vec2 &box_br, const Vec2 &box_tl)
{

}
*/





} // namespace MATH
} // namespace MO
