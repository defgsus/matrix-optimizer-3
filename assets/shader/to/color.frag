#include <to/header>
#include <constants>

uniform sampler2D   u_tex;
uniform vec4        u_color;
uniform vec3        u_hsv;

void main(void)
{
    vec4 col = texture(u_tex, v_texCoord.xy);

    // desaturate
    col.xyz += (1. - u_hsv.y) * (dot(col.xyz, vec3(.25, .6, .15)) - col.xyz);

    fragColor = col * u_color;
}
