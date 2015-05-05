#include <rotate>

/* Creates a translation matrix */
mat4 translate4(in vec3 vec);
mat4 translate(in vec3 vec) { return translate4(vec); }

/* Translates a matrix */
mat4 translate4(in mat4 m, in vec3 vec);
mat4 translate(in mat4 m, in vec3 vec) { return translate4(m, vec); }

/* Scales a 3x3 matrix */
mat3 scale3(in mat3 m, in float s);
mat3 scale(in mat3 m, in float s) { return scale3(m, s); }

/* Scales a 4x4 matrix */
mat4 scale4(in mat4 m, in float s);
mat4 scale(in mat4 m, in float s) { return scale4(m, s); }

/* Creates a scaling matrix */
mat3 scale3(in float s);
mat4 scale4(in float s);
/* default is 4x4 */
mat4 scale(in float s) { return scale4(s); }




mat4 translate4(in vec3 vec)
{
        mat4 m = mat4(1.);
        m[3] += m * vec4(vec, 0.);
        return m;
}

mat4 translate4(in mat4 m, in vec3 vec)
{
        m[3] += m * vec4(vec, 0.);
        return m;
}

mat3 scale3(in float s)
{
        return mat3(vec3(s,0.,0.), vec3(0.,s,0.), vec3(0.,0.,s));
}

mat4 scale4(in float s)
{
        return mat4(vec4(s,0.,0.,0.), vec4(0.,s,0.,0.), vec4(0.,0.,s,0.), vec4(0.,0.,0.,1.));
}

mat3 scale3(in mat3 m, in float s)
{
        return m * s;
}

mat4 scale4(in mat4 m, in float s)
{
        m[0] *= s;
        m[1] *= s;
        m[2] *= s;
        return m;
}
