#ifndef EPSILON
#   define EPSILON (1e-6)
#endif

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
    or the distance between ro and the intersection point. */
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
