#include <constants>

/** Direction -> equi-rectangular texture lookup.
    Output is in range [-1, 1] */
vec2 dir2equirect(in vec3 dir)
{
    return vec2(atan(dir.x, -dir.z) / PI,
                1. - 2. * acos(dir.y) / PI);
}
