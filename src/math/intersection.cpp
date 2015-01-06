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


/** Build after @ref http://geomalgorithms.com/a06-_intersect-2.html#intersect3D_RayTriangle%28%29 */
bool intersect_ray_triangle(const Vec3& ray_origin,
                            const Vec3& ray_direction,
                            const Vec3& v0,
                            const Vec3& v1,
                            const Vec3& v2,
                            Vec3 * intersect_pos)
{
    Vec3 u, v, n;           // triangle vectors
    Vec3 w0, w;             // ray vectors
    Float r, a, b;          // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = v1 - v0;
    v = v2 - v0;
    n = glm::cross(u, v);
    // triangle is degenerate?
    if (n.x == 0.f && n.y == 0.f && n.z == 0.f)
        return false;

    w0 = ray_origin - v0;
    a = -glm::dot(n, w0);
    b = glm::dot(n, ray_direction);
    if (std::abs(b) < std::numeric_limits<Float>::epsilon())
        return false;

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)            // ray goes away from triangle
        return false;

    // intersect point of ray and plane
    *intersect_pos = ray_origin + r * ray_direction;

    // is I inside T?
    Float uu, uv, vv, wu, wv, D;
    uu = glm::dot(u,u);
    uv = glm::dot(u,v);
    vv = glm::dot(v,v);
    w = *intersect_pos - v0;
    wu = glm::dot(w,u);
    wv = glm::dot(w,v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    Float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.f || s > 1.f)
        return false;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.f || (s + t) > 1.f)
        return false;

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










} // namespace MATH
} // namespace MO
