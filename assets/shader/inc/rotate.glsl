

/* Rotates a 2d vector in radians */
vec2 rotate(in vec2 v, in float rad);

/* Rotates a 3d vector around an axis in radians */
vec3 rotateX(in vec3 v, in float rad);
vec3 rotateY(in vec3 v, in float rad);
vec3 rotateZ(in vec3 v, in float rad);
vec3 rotateAxis(in vec3 v, in vec3 ax, in float rad);

/* Creates a 3x3 rotation matrix around an axis in radians */
mat3 rotateX3(in float rad);
mat3 rotateY3(in float rad);
mat3 rotateZ3(in float rad);
mat3 rotateAxis3(in vec3 axis, in float rad);

/* Creates a 4x4 rotation matrix around an axis in radians */
mat4 rotateX4(in float rad);
mat4 rotateY4(in float rad);
mat4 rotateZ4(in float rad);
mat4 rotateAxis4(in vec3 axis, in float rad);

/* default is 4x4 */
mat4 rotateX(in float rad) { return rotateX4(rad); }
mat4 rotateY(in float rad) { return rotateY4(rad); }
mat4 rotateZ(in float rad) { return rotateZ4(rad); }
mat4 rotateAxis(in vec3 axis, in float rad) { return rotateAxis4(axis, rad); }

/* Rotates a 3x3 matrix around an axis in radians */
mat3 rotateX3(in mat3 m, in float rad) { return m * rotateX3(rad); }
mat3 rotateY3(in mat3 m, in float rad) { return m * rotateY3(rad); }
mat3 rotateZ3(in mat3 m, in float rad) { return m * rotateZ3(rad); }
mat3 rotateAxis3(in mat3 m, in vec3 axis, in float rad) { return m * rotateAxis3(axis, rad); }

mat3 rotateX(in mat3 m, in float rad) { return m * rotateX3(rad); }
mat3 rotateY(in mat3 m, in float rad) { return m * rotateY3(rad); }
mat3 rotateZ(in mat3 m, in float rad) { return m * rotateZ3(rad); }
mat3 rotateAxis(in mat3 m, in vec3 axis, in float rad) { return m * rotateAxis3(axis, rad); }

/* Rotates a 4x4 matrix around an axis in radians */
mat4 rotateX4(in mat4 m, in float rad) { return m * rotateX4(rad); }
mat4 rotateY4(in mat4 m, in float rad) { return m * rotateY4(rad); }
mat4 rotateZ4(in mat4 m, in float rad) { return m * rotateZ4(rad); }
mat4 rotateAxis4(in mat4 m, in vec3 axis, in float rad) { return m * rotateAxis4(axis, rad); }

mat4 rotateX(in mat4 m, in float rad) { return m * rotateX4(rad); }
mat4 rotateY(in mat4 m, in float rad) { return m * rotateY4(rad); }
mat4 rotateZ(in mat4 m, in float rad) { return m * rotateZ4(rad); }
mat4 rotateAxis(in mat4 m, in vec3 axis, in float rad) { return m * rotateAxis4(axis, rad); }


// -------------------- vector rotations ----------------------

/** Rotates a 2d vector */
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
                     v.y,
                    -v.x * sa + v.z * ca);
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


// ------------------------ matrix rotations ------------------------

mat3 rotateAxis3(in vec3 axis, in float rad)
{
        float c = cos(rad), s = sin(rad);

        vec3 temp = (1. - c) * axis;

        mat3 rot = mat3(vec3(c + temp.x * axis.x,
                                                     temp.x * axis.y + s * axis.z,
                             temp.x * axis.z - s * axis.y),

                                        vec3(    temp.y * axis.x - s * axis.z,
                                                 c + temp.y * axis.y,
                                                     temp.y * axis.z + s * axis.x),

                                        vec3(    temp.z * axis.x + s * axis.y,
                                                         temp.z * axis.y - s * axis.x,
                                                 c + temp.z * axis.z));
        return rot;
}

mat4 rotateAxis4(in vec3 axis, in float rad)
{
        float c = cos(rad), s = sin(rad);

        vec3 temp = (1. - c) * axis;

        mat4 rot = mat4(vec4(c + temp.x * axis.x,
                                                     temp.x * axis.y + s * axis.z,
                             temp.x * axis.z - s * axis.y, 0.),

                                        vec4(    temp.y * axis.x - s * axis.z,
                                                 c + temp.y * axis.y,
                                                     temp.y * axis.z + s * axis.x, 0.),

                                        vec4(    temp.z * axis.x + s * axis.y,
                                                         temp.z * axis.y - s * axis.x,
                                                 c + temp.z * axis.z, 0.),
                                        vec4(0., 0., 0., 1.));
        return rot;
}

/** Creates a matrix with a rotation around the x-axis in radians */
mat3 rotateX3(in float rad)
{
        float c = cos(rad), s = sin(rad);
        return mat3(vec3(1., 0., 0.),
                                vec3(0.,  c, -s),
                                vec3(0.,  s,  c));
}

/** Creates a matrix with a rotation around the y-axis in radians */
mat3 rotateY3(in float rad)
{
        float c = cos(rad), s = sin(rad);
        return mat3(vec3( c, 0.,  s),
                                vec3(0., 1., 0.),
                                vec3(-s, 0.,  c));
}

/** Creates a matrix with a rotation around the z-axis in radians */
mat3 rotateZ3(in float rad)
{
        float c = cos(rad), s = sin(rad);
        return mat3(vec3( c, -s, 0.),
                                vec3( s,  c, 0.),
                                vec3(0., 0., 1.));
}


/** Creates a matrix with a rotation around the x-axis in radians */
mat4 rotateX4(in float rad)
{
        float c = cos(rad), s = sin(rad);
        return mat4(vec4(1., 0., 0., 0.),
                                vec4(0.,  c, -s, 0.),
                                vec4(0.,  s,  c, 0.),
                                vec4(0., 0., 0., 1.));
}

/** Creates a matrix with a rotation around the y-axis in radians */
mat4 rotateY4(in float rad)
{
        float c = cos(rad), s = sin(rad);
        return mat4(vec4( c, 0.,  s, 0.),
                                vec4(0., 1., 0., 0.),
                                vec4(-s, 0.,  c, 0.),
                                vec4(0., 0., 0., 1.));
}

/** Creates a matrix with a rotation around the z-axis in radians */
mat4 rotateZ4(in float rad)
{
        float c = cos(rad), s = sin(rad);
        return mat4(vec4( c, -s, 0., 0.),
                                vec4( s,  c, 0., 0.),
                                vec4(0., 0., 1., 0.),
                                vec4(0., 0., 0., 1.));
}

