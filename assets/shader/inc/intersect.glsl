#include <constants>
#include <matrix>

/** Returns intersection position along the ray ro -> rd
    for a plane with normal plane.xyz and position plane.xyz * plane.w.
    Result is either negative for no hit
    or the distance between ro and the intersection point. */
float intersect_plane(in vec3 ro, in vec3 rd, in vec4 plane)
{
    float dir = dot(plane.xyz, rd);
    if (abs(dir) < EPSILON) return -1.;
    return dot(plane.xyz, plane.xyz * plane.w - ro) / dir;
}


/** Returns intersection position along the ray ro -> rd
    for a sphere at sphere.xyz with radius sphere.w.
    Result is either negative for no hit
    or the distance between ro and the intersection point.

    From PovRay implementation.
*/
float intersect_sphere(in vec3 ro, in vec3 rd, in vec4 sphere)
{
    vec3 oc = sphere.xyz - ro;
    float oc2 = dot(oc, oc);
    float closest = dot(oc, rd);
    float radius2 = sphere.w * sphere.w;

    if (oc2 >= radius2 && closest < EPSILON)
        return -1.;

    float halfc2 = radius2 - oc2 + closest * closest;

    if (halfc2 < EPSILON)
        return -1.;

    float halfc = sqrt(halfc2);
    return oc2 < radius2 ? closest + halfc : closest - halfc;
}


/** Returns intersection position along the ray ro -> rd
    for an endless tube at tube.xyz and radius tube.w,
    expanding along the given axis.
    Result is either negative for no hit
    or the distance between ro and the intersection point.

    From http://www.geometrictools.com/GTEngine/Include/GteIntrLine3Cylinder3.h
    */
float intersect_tube(in vec3 ro, in vec3 rd, vec4 tube, vec3 axis)
{
    float dz = dot(axis, rd);
    // parallel?
    if (abs(dz) == 1.)
            return -1.;

    mat3 basis = compute_orthogonals(axis);

    float rad2 = tube.w * tube.w;

    vec3 diff = ro - tube.xyz;
    vec3 P = vec3(dot(basis[1], diff), dot(basis[2], diff), dot(basis[0], diff));

    vec3 D = vec3(dot(basis[1], rd), dot(basis[2], rd), dz);

    float a0 = P[0] * P[0] + P[1] * P[1] - rad2;
    float a1 = P[0] * D[0] + P[1] * D[1];
    float a2 = D[0] * D[0] + D[1] * D[1];
    float dr = a1 * a1 - a0 * a2;
    if (dr > 0.)
    {
            float r = sqrt(dr);
            return a0 > 0. ? (-a1 - r) / a2 : (-a1 + r) / a2;
    }
    return -1.;
}
