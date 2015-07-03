#include <to/header>

uniform sampler3D   u_tex;
uniform float       u_slice_n;      // normalized [0,1] slice position

//%mo_user_code%

void main(void)
{
    vec3 pos = vec3(v_texCoord, u_slice_n);

    fragColor = texture_pixel(pos);
}
