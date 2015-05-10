#include <to/header>
#include <constants>
#include <color>

uniform sampler2D   u_tex;
uniform vec4        u_color;
uniform vec3        u_hsv;
uniform vec3        u_gamma_exp;

void main(void)
{
    vec4 col = texture(u_tex, v_texCoord.xy);

    // hsv
    vec3 hsv = rgb2hsv(col.xyz);
    hsv.x += u_hsv.x;
    col.xyz = hsv2rgb(hsv);

    // rgb multiply
    col *= u_color;

    // desaturate
    col.xyz += (1. - u_hsv.y) * (dot(col.xyz, vec3(.25, .6, .15)) - col.xyz);

    // gamma
    col.xyz = pow(col.xyz, u_gamma_exp) * u_hsv.z;

    fragColor = col;
}
