#include <constants>
// for compute_orthogonals()
#include <matrix>


struct Ray
{
    // Ray origin
    vec3 ro,
    // Ray direction
        rd,
    // surface hit position
        hit,
    // surface hit normal
        n,
    // color accumulator
        color;
    // length travelled
    float t,
    // object id or user field
        id;
};

Ray makeRay(in vec3 ro, in vec3 rd)
{
    return Ray(ro, rd, vec3(0.), vec3(0.), vec3(0.), 0., 0.);
}

bool intersectPlane(
                inout Ray r,
                in vec3 plane_norm, vec3 plane_pos = vec3(0.))
{
    float dir = dot(plane_norm, r.rd);
        // parallel to plane?
    if (abs(dir) < EPSILON) return false;

    r.t = dot(plane_norm, plane_pos - r.ro) / dir;
    if (r.t > 0.)
    {
        r.hit = r.ro + r.t * r.rd;
        r.n = plane_norm;
        return true;
    }
    return false;
}

/*
    From PovRay implementation.
*/
bool intersectSphere(inout Ray r, in vec4 sphere)
{
    vec3 oc = sphere.xyz - r.ro;
    float oc2 = dot(oc, oc);
    float closest = dot(oc, r.rd);
    float radius2 = sphere.w * sphere.w;

    if (oc2 >= radius2 && closest < EPSILON)
        return false;

    float halfc2 = radius2 - oc2 + closest * closest;

    if (halfc2 < EPSILON)
        return false;

    float halfc = sqrt(halfc2);
    r.t = (oc2 < radius2) ? closest + halfc : closest - halfc;
    r.hit = r.ro + r.t * r.rd;
    r.n = normalize(r.hit - sphere.xyz);
    return true;
}

bool intersectSphere(inout Ray r, in vec3 sphere, in float radius)
{
    return intersectSphere(r, vec4(sphere, radius));
}

/** Returns intersection position along the ray ro -> rd
    for an endless tube at tube.xyz and radius tube.w,
    expanding along the given axis.

    From http://www.geometrictools.com/GTEngine/Include/GteIntrLine3Cylinder3.h
    */
bool intersectTube(
                inout Ray r,
                in vec4 tube, in vec3 axis = vec3(0., 1., 0.))
{
    float dz = dot(axis, r.rd);
    // parallel?
    if (abs(dz) == 1.)
            return false;

    mat3 basis = compute_orthogonals(axis);

    float rad2 = tube.w * tube.w;

    vec3 diff = r.ro - tube.xyz;
    vec3 P = vec3(
                dot(basis[1], diff), dot(basis[2], diff), dot(basis[0], diff));
    vec3 D = vec3(
                dot(basis[1], r.rd), dot(basis[2], r.rd), dz);

    float a0 = P.x * P.x + P.y * P.y - rad2;
    float a1 = P.x * D.x + P.y * D.y;
    float a2 = D.x * D.x + D.y * D.y;
    float dr = a1 * a1 - a0 * a2;
    if (dr > 0.)
    {
        float sr = sqrt(dr);
        r.t = a0 > 0. ? (-a1 - sr) / a2 : (-a1 + sr) / a2;
        if (r.t > 0.)
        {
            r.hit = r.ro + r.t * r.rd;
            // XXX Todo: normal not right when not axis aligned
            vec3 aaxis = 1. - abs(axis);
            r.n = normalize((r.hit - tube.xyz) * aaxis);
            return true;
        }
    }
    return false;
}
bool intersectTube(
                inout Ray r,
                vec3 tube, in float radius, vec3 axis = vec3(0., 1., 0.))
{
    return intersectTube(r, vec4(tube, radius), axis);
}

// http://www.gamedev.net/topic/495636-raybox-collision-intersection-point/
bool intersectBox(inout Ray r, vec3 box, vec3 size)
{
    vec3 tmin = ((box-size) - r.ro) / r.rd;
    vec3 tmax = ((box+size) - r.ro) / r.rd;

    vec3 real_min = min(tmin, tmax);
    vec3 real_max = max(tmin, tmax);

    float minmax = min( min(real_max.x, real_max.y), real_max.z);
    float maxmin = max( max(real_min.x, real_min.y), real_min.z);

    if (minmax >= maxmin && minmax > 0.)
    {
        r.t = maxmin > 0 ? maxmin : minmax;
        r.hit = r.ro + r.t * r.rd;
        // find normal
        vec3 n = r.hit - box, a = abs(n);

        r.n = a.x > a.y ? a.x > a.z
                ? vec3(sign(n.x),0.,0.) : a.z > a.y
                        ? vec3(0.,0.,sign(n.z)) : vec3(0.,sign(n.y),0.) : a.y > a.z
                                ? vec3(0.,sign(n.y),0.) : vec3(0.,0.,sign(n.z));

        return true;
    }
    return false;
}
