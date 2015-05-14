#include <to/header>

// define KERNEL 0-3
// define SIGNED 0,1
// define MUL_APHA 0,1

uniform sampler2D   u_tex;
uniform vec2        u_eps;
uniform vec4        u_color;

float texel(in vec2 uv)
{
#if KERNEL == 0
    vec4 t = texture(u_tex, uv);

#elif KERNEL == 1
    float eps = u_eps.y / 2.;
    vec4 t = vec4(0.);

    t += texture(u_tex, uv + vec2(-1., -1.) * eps) * 0.0666666666667;
    t += texture(u_tex, uv + vec2(0., -1.) * eps) * 0.133333333333;
    t += texture(u_tex, uv + vec2(1., -1.) * eps) * 0.0666666666667;
    t += texture(u_tex, uv + vec2(-1., 0.) * eps) * 0.133333333333;
    t += texture(u_tex, uv + vec2(0., 0.) * eps) * 0.2;
    t += texture(u_tex, uv + vec2(1., 0.) * eps) * 0.133333333333;
    t += texture(u_tex, uv + vec2(-1., 1.) * eps) * 0.0666666666667;
    t += texture(u_tex, uv + vec2(0., 1.) * eps) * 0.133333333333;
    t += texture(u_tex, uv + vec2(1., 1.) * eps) * 0.0666666666667;

#elif KERNEL == 2
    float eps = u_eps.y / 4.;
    vec4 t = vec4(0.);

    t += texture(u_tex, uv + vec2(-2., -2.) * eps) * 0.0153846153846;
    t += texture(u_tex, uv + vec2(-1., -2.) * eps) * 0.0307692307692;
    t += texture(u_tex, uv + vec2(0., -2.) * eps) * 0.0461538461538;
    t += texture(u_tex, uv + vec2(1., -2.) * eps) * 0.0307692307692;
    t += texture(u_tex, uv + vec2(2., -2.) * eps) * 0.0153846153846;
    t += texture(u_tex, uv + vec2(-2., -1.) * eps) * 0.0307692307692;
    t += texture(u_tex, uv + vec2(-1., -1.) * eps) * 0.0461538461538;
    t += texture(u_tex, uv + vec2(0., -1.) * eps) * 0.0615384615385;
    t += texture(u_tex, uv + vec2(1., -1.) * eps) * 0.0461538461538;
    t += texture(u_tex, uv + vec2(2., -1.) * eps) * 0.0307692307692;
    t += texture(u_tex, uv + vec2(-2., 0.) * eps) * 0.0461538461538;
    t += texture(u_tex, uv + vec2(-1., 0.) * eps) * 0.0615384615385;
    t += texture(u_tex, uv + vec2(0., 0.) * eps) * 0.0769230769231;
    t += texture(u_tex, uv + vec2(1., 0.) * eps) * 0.0615384615385;
    t += texture(u_tex, uv + vec2(2., 0.) * eps) * 0.0461538461538;
    t += texture(u_tex, uv + vec2(-2., 1.) * eps) * 0.0307692307692;
    t += texture(u_tex, uv + vec2(-1., 1.) * eps) * 0.0461538461538;
    t += texture(u_tex, uv + vec2(0., 1.) * eps) * 0.0615384615385;
    t += texture(u_tex, uv + vec2(1., 1.) * eps) * 0.0461538461538;
    t += texture(u_tex, uv + vec2(2., 1.) * eps) * 0.0307692307692;
    t += texture(u_tex, uv + vec2(-2., 2.) * eps) * 0.0153846153846;
    t += texture(u_tex, uv + vec2(-1., 2.) * eps) * 0.0307692307692;
    t += texture(u_tex, uv + vec2(0., 2.) * eps) * 0.0461538461538;
    t += texture(u_tex, uv + vec2(1., 2.) * eps) * 0.0307692307692;
    t += texture(u_tex, uv + vec2(2., 2.) * eps) * 0.0153846153846;

#elif KERNEL == 3
    float eps = u_eps.y / 6.;
    vec4 t = vec4(0.);

    t += texture(u_tex, uv + vec2(-3., -3.) * eps) * 0.00571428571429;
    t += texture(u_tex, uv + vec2(-2., -3.) * eps) * 0.0114285714286;
    t += texture(u_tex, uv + vec2(-1., -3.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(0., -3.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(1., -3.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(2., -3.) * eps) * 0.0114285714286;
    t += texture(u_tex, uv + vec2(3., -3.) * eps) * 0.00571428571429;
    t += texture(u_tex, uv + vec2(-3., -2.) * eps) * 0.0114285714286;
    t += texture(u_tex, uv + vec2(-2., -2.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(-1., -2.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(0., -2.) * eps) * 0.0285714285714;
    t += texture(u_tex, uv + vec2(1., -2.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(2., -2.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(3., -2.) * eps) * 0.0114285714286;
    t += texture(u_tex, uv + vec2(-3., -1.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(-2., -1.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(-1., -1.) * eps) * 0.0285714285714;
    t += texture(u_tex, uv + vec2(0., -1.) * eps) * 0.0342857142857;
    t += texture(u_tex, uv + vec2(1., -1.) * eps) * 0.0285714285714;
    t += texture(u_tex, uv + vec2(2., -1.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(3., -1.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(-3., 0.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(-2., 0.) * eps) * 0.0285714285714;
    t += texture(u_tex, uv + vec2(-1., 0.) * eps) * 0.0342857142857;
    t += texture(u_tex, uv + vec2(0., 0.) * eps) * 0.04;
    t += texture(u_tex, uv + vec2(1., 0.) * eps) * 0.0342857142857;
    t += texture(u_tex, uv + vec2(2., 0.) * eps) * 0.0285714285714;
    t += texture(u_tex, uv + vec2(3., 0.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(-3., 1.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(-2., 1.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(-1., 1.) * eps) * 0.0285714285714;
    t += texture(u_tex, uv + vec2(0., 1.) * eps) * 0.0342857142857;
    t += texture(u_tex, uv + vec2(1., 1.) * eps) * 0.0285714285714;
    t += texture(u_tex, uv + vec2(2., 1.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(3., 1.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(-3., 2.) * eps) * 0.0114285714286;
    t += texture(u_tex, uv + vec2(-2., 2.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(-1., 2.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(0., 2.) * eps) * 0.0285714285714;
    t += texture(u_tex, uv + vec2(1., 2.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(2., 2.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(3., 2.) * eps) * 0.0114285714286;
    t += texture(u_tex, uv + vec2(-3., 3.) * eps) * 0.00571428571429;
    t += texture(u_tex, uv + vec2(-2., 3.) * eps) * 0.0114285714286;
    t += texture(u_tex, uv + vec2(-1., 3.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(0., 3.) * eps) * 0.0228571428571;
    t += texture(u_tex, uv + vec2(1., 3.) * eps) * 0.0171428571429;
    t += texture(u_tex, uv + vec2(2., 3.) * eps) * 0.0114285714286;
    t += texture(u_tex, uv + vec2(3., 3.) * eps) * 0.00571428571429;

#endif

#if MUL_ALPHA
    t.rgb *= t.a;
#endif
    return dot(t, u_color);
}



void main()
{
    vec2 eps = vec2(u_eps.x, 0);
    vec2 uv = v_texCoord;

    vec3 n = normalize(vec3(
                        texel(uv - eps.xy) - texel(uv + eps.xy),
                        texel(uv - eps.yx) - texel(uv + eps.yx),
                        2. * eps.x));

#if SIGNED == 0
    fragColor = vec4(n * .5 + .5, 1.);
#else
    fragColor = vec4(n, 1.);
#endif
}
