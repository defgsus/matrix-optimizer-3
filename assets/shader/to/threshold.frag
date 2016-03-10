#include <to/header>

// define INVERT 0,1

uniform vec4        u_settings; // threshold, .., .., ..
uniform vec4        u_color;

void main()
{
    float t = smoothstep(u_settings.x, u_settings.x+0.0001,
                         dot(texture(u_tex, v_texCoord), u_color)
                         );
#if INVERT == 1
    t = 1. - t;
#endif
    fragColor = vec4(t, t, t, 1.);
}
