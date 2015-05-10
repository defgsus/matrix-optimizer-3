#include <to/header>

#ifdef HSV_COMPARE
#include <color>
#endif

uniform sampler2D   u_tex;
uniform vec3        u_color;
uniform vec2        u_range;
uniform vec2        u_mix;

void main(void)
{
    vec4 col = texture(u_tex, v_texCoord.xy);

#ifndef HSV_COMPARE
    float diff = length(col.xyz - u_color) / sqrt(3.);
#else
    float diff = length(rgb2hsv(col.xyz) - u_color) / sqrt(3.);
#endif

    float alpha = smoothstep(u_range.x, u_range.y, 1. - diff);

    // mix-in alpha
    alpha += u_mix.x * (alpha * col.w - alpha);
    // mix towards white
    col.xyz += u_mix.y * (vec3(1.) - col.xyz);

    fragColor = vec4(col.xyz, alpha);
}
