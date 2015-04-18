
/** Rotate a 2d vector */
vec2 rotate(in vec2 v, in float rad)
{
    float s = sin(rad), c = cos(rad);
    return vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

/** Rotate a 3d vector around the x-axis */
vec3 rotateX(in vec3 v, in float rad)
{
    float sa = sin(rad), ca = cos(rad);
    return vec3(    v.x,
                    v.y * ca - v.z * sa,
                    v.y * sa + v.z * ca);
}

/** Rotate a 3d vector around the y-axis */
vec3 rotateY(in vec3 v, in float rad)
{
    float sa = sin(rad), ca = cos(rad);
    return vec3(     v.x * ca + v.z * sa,
                    -v.x * sa + v.z * ca,
                     v.z);
}

/** Rotate a 3d vector around the z-axis */
vec3 rotateZ(in vec3 v, in float rad)
{
    float sa = sin(rad), ca = cos(rad);
    return vec3(    v.x * ca - v.y * sa,
                    v.x * sa + v.y * ca,
                    v.z);
}

/** Rotate vector @p v around the axis @p ax, for @p rad radians. */
vec3 rotateAxis(in vec3 v, in vec3 ax, in float rad)
{
    float   SI = sin(rad), CO = cos(rad),
            m = ax.x*ax.x + ax.y*ax.y + ax.z*ax.z,
            ms = sqrt(m);

    return vec3(
            (ax.x* (ax.x * v.x + ax.y * v.y + ax.z * v.z)
                + CO * ( v.x*(ax.y*ax.y + ax.z*ax.z) + ax.x*(-ax.y*v.y - ax.z*v.z) )
                + SI * ms * (-ax.z * v.y + ax.y * v.z) ) / m,
            (ax.y* (ax.x * v.x + ax.y * v.y + ax.z * v.z)
                + CO * ( v.y*(ax.x*ax.x + ax.z*ax.z) + ax.y*(-ax.x*v.x - ax.z*v.z) )
                + SI * ms * ( ax.z * v.x - ax.x * v.z) ) / m,
            (ax.z* (ax.x * v.x + ax.y * v.y + ax.z * v.z)
                + CO * ( v.z*(ax.x*ax.x + ax.y*ax.y) + ax.z*(-ax.x*v.x - ax.y*v.y) )
                + SI * ms * (-ax.y * v.x + ax.x * v.y) ) / m );
}
