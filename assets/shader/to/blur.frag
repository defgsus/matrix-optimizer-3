#include <to/header>
#include <constants>

// define USE_MASK 0,1
// define USE_SIZE3 0,1

uniform vec2        u_size_sigma;   // size, smoothness
uniform vec3        u_size3;        // three sizes
uniform vec2        u_direction;    // (1,0) or (0,1)
uniform float       u_num;          // number samples
uniform float       u_alpha;        // output alpha
uniform vec3        u_mask_range;

/* http://callumhay.blogspot.de/2010/09/gaussian-blur-shader-glsl.html */

void main()
{
    float sigma = u_size_sigma.y,
          size = u_size_sigma.x,
          sum = 0.;

#if USE_MASK
    sigma = max(0.001,
                sigma * smoothstep(u_mask_range.x, u_mask_range.y,
                                   abs(texture(u_tex_mask, v_texCoord.xy).x - u_mask_range.z))
                );
#endif

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

    c /= sum;

    fragColor = clamp(vec4(1.,1.,1., u_alpha) * (
#if MO_ALPHA_MODE == 1
        vec4(c.xyz, 1.)
#elif MO_ALPHA_MODE == 2
        vec4(c.xyz, length(c.xyz) / sqrt(3.))
#else
        c
#endif

        ), 0., 1.);
}
