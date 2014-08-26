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

        *depth1 = closest + half_chord;
        *depth2 = closest - half_chord;

        return true;
    }

    return false;
}

/*
bool Sphere::Intersect(const Ray& ray, const VECTOR Center, DBL Radius2, DBL *Depth1, DBL *Depth2)
{
    DBL OCSquared, t_Closest_Approach, Half_Chord, t_Half_Chord_Squared;
    VECTOR Origin_To_Center;

    VSub(Origin_To_Center, Center, ray.Origin);

    VDot(OCSquared, Origin_To_Center, Origin_To_Center);

    VDot(t_Closest_Approach, Origin_To_Center, ray.Direction);

    if ((OCSquared >= Radius2) && (t_Closest_Approach < EPSILON))
        return(false);

    t_Half_Chord_Squared = Radius2 - OCSquared + Sqr(t_Closest_Approach);

    if (t_Half_Chord_Squared > EPSILON)
    {
        Half_Chord = sqrt(t_Half_Chord_Squared);

        *Depth1 = t_Closest_Approach - Half_Chord;
        *Depth2 = t_Closest_Approach + Half_Chord;

        return(true);
    }

    return(false);
}
*/

} // namespace MATH
} // namespace MO
