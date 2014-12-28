/** @file intersection.cpp

    @brief Geometric intersection functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include <glm/gtx/intersect.hpp>

#include "intersection.h"

namespace MO {
namespace MATH {

namespace { const Float EPSILON = 0.00001; }


bool inside_range(const Vec2 &v, Float mi, Float ma)
{
    return v[0] >= mi && v[1] >= mi && v[0] <= ma && v[1] <= ma;
}

bool inside_range(const Vec2 &v, const Vec2& mi, const Vec2& ma)
{
    return v[0] >= mi[0] && v[1] >= mi[1] && v[0] <= ma[0] && v[1] <= ma[1];
}


Vec3 closest_point_on_line(const Vec3 &point, const Vec3 &lineA, const Vec3 &lineB)
{
    return glm::closestPointOnLine(point, lineA, lineB);
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


bool intersect_ray_triangle(const Vec3& ray_origin,
                            const Vec3& ray_direction,
                            const Vec3& vert0,
                            const Vec3& vert1,
                            const Vec3& vert2,
                            Vec3 * intersect_pos)
{
    // XXX Returned position is very bogus
    if (intersect_pos)
    {
        return glm::intersectRayTriangle(ray_origin, ray_direction,
                                         vert0, vert1, vert2, *intersect_pos);
    }
    else
    {
        Vec3 dummy;
        return glm::intersectRayTriangle(ray_origin, ray_direction,
                                         vert0, vert1, vert2, dummy);
    }
}

#ifdef _returned_position_seems_to_be_totally_meaningless_to_me_

bool intersect_ray_triangle(const Vec3& ray_origin,
                            const Vec3& ray_direction,
                            const Vec3& vert0,
                            const Vec3& vert1,
                            const Vec3& vert2,
                            Vec3 * intersect_pos)
{
    Float Epsilon = std::numeric_limits<Float>::epsilon();

    Vec3 edge1 = vert1 - vert0;
    Vec3 edge2 = vert2 - vert0;

    Vec3 pvec = glm::cross(ray_direction, edge2);

    Float det = glm::dot(edge1, pvec);
    if (det < Epsilon)
        return false;

    Vec3 position,
         tvec = ray_origin - vert0;

    position.y = glm::dot(tvec, pvec);
    if (position.y < 0.f || position.y > det)
        return false;

    Vec3 qvec = glm::cross(tvec, edge1);

    position.z = glm::dot(ray_direction, qvec);
    if (position.z < 0.f || position.y + position.z > det)
        return false;

    position.x = glm::dot(edge2, qvec);
    position /= det;

    if (intersect_pos)
        *intersect_pos = position;

    return true;
}

#endif

#ifdef _attempt_to_copy_the_povray_source_was_also_not_working_
bool intersect_ray_triangle(const Vec3& ray_origin,
                            const Vec3& ray_direction,
                            const Vec3& P1,
                            const Vec3& P2,
                            const Vec3& P3,
                            Vec3 * intersect_pos)
{
    Vec3 triNormal = glm::normalize(glm::cross(P2 - P1, P3 - P1));

    Float normalDotDirection = glm::dot(triNormal, ray_direction);

    if (std::abs(normalDotDirection) < std::numeric_limits<Float>::epsilon())
        return false;

    Float normalDotOrigin = glm::dot(triNormal, ray_origin),
          distance = -glm::dot(triNormal, P1);

    Float depth = -(distance + normalDotOrigin) / normalDotDirection;

    if (depth < std::numeric_limits<Float>::epsilon()
       || depth > 1.f)
        return false;

    Vec3 absTriNorm = triNormal * triNormal;
    int dominantAxis = 0;
    if (absTriNorm.y > absTriNorm.x && absTriNorm.y > absTriNorm.z)
        dominantAxis = 1;
    else
    if (absTriNorm.z > absTriNorm.x && absTriNorm.z > absTriNorm.y)
        dominantAxis = 2;

    Float s, t;
    switch (dominantAxis)
    {
        case 0:
            s = ray_origin.y + depth * ray_direction.y;
            t = ray_origin.z + depth * ray_direction.z;

            if ((P2.y - s) * (P2.z - P1.z) <
                (P2.z - t) * (P2.y - P1.y))
            {
                return false;
            }

            if ((P3.y - s) * (P3.z - P2.z) <
                (P3.z - t) * (P3.y - P2.y))
            {
                return false;
            }

            if ((P1.y - s) * (P1.z - P3.z) <
                (P1.z - t) * (P1.y - P3.y))
            {
                return false;
            }

            if (intersect_pos)
                *intersect_pos = ray_origin + depth * ray_direction;
            return true;

        case 1:

            s = ray_origin.x + depth * ray_direction.x;
            t = ray_origin.z + depth * ray_direction.z;

            if ((P2.x - s) * (P2.z - P1.z) <
                (P2.z - t) * (P2.x - P1.x))
            {
                return false;
            }

            if ((P3.x - s) * (P3.z - P2.z) <
                (P3.z - t) * (P3.x - P2.x))
            {
                return false;
            }

            if ((P1.x - s) * (P1.z - P3.z) <
                (P1.z - t) * (P1.x - P3.x))
            {
                return false;
            }

            if (intersect_pos)
                *intersect_pos = ray_origin + depth * ray_direction;
            return true;

        case 2:

            s = ray_origin.x + depth * ray_direction.x;
            t = ray_origin.y + depth * ray_direction.y;

            if ((P2.x - s) * (P2.y - P1.y) <
                (P2.y - t) * (P2.x - P1.x))
            {
                return false;
            }

            if ((P3.x - s) * (P3.y - P2.y) <
                (P3.y - t) * (P3.x - P2.x))
            {
                return false;
            }

            if ((P1.x - s) * (P1.y - P3.y) <
                (P1.y - t) * (P1.x - P3.x))
            {
                return false;
            }

            if (intersect_pos)
                *intersect_pos = ray_origin + depth * ray_direction;
            return true;
    }
    return false;
}
#endif




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










} // namespace MATH
} // namespace MO
