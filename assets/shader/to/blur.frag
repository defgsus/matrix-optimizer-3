#include <to/header>
#include <constants>

uniform sampler2D   u_tex;          // input texture
uniform vec2        u_size_sigma;   // size, smoothness
uniform vec2        u_direction;    // (1,0) or (0,1)
uniform float       u_num;          // number samples

/* http://callumhay.blogspot.de/2010/09/gaussian-blur-shader-glsl.html */

void main()
{
    float sigma = u_size_sigma.y,
          size = u_size_sigma.x,
          sum;
    vec3 inc;
    inc.x = 1. / (sqrt(TAU) * sigma);
    inc.y = exp(-.5 / (sigma * sigma));
    inc.z = inc.y * inc.y;

    vec2 uv = v_texCoord.xy;
    vec4 c = texture(u_tex, uv) * inc.x;
    sum += inc.x;
    inc.xy *= inc.yz;

    vec2 stp = u_direction * size;
    for (float i=1.; i<u_num; ++i)
    {
        c += texture(u_tex, uv + stp * i) * inc.x;
        c += texture(u_tex, uv - stp * i) * inc.x;
        sum += 2. * inc.x;
        inc.xy *= inc.yz;
    }

    fragColor = c / sum;
}
