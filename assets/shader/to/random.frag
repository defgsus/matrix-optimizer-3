#include <to/header>
#include <constants>
#include <rotate>
#include <color>
#include <dh/hash>
#include <noise>
#include <iq/vnoise>

//define RANDOM_ROTATE 0,1
//define NOISE_FUNC 0,1,2
//define MASK 0,1
//define USE_ALPHA 0,1
//define FRACTAL_MODE 0,1,2,3

uniform vec4        u_color;
uniform vec3        u_hsv;
uniform vec3        u_gamma_exp;
uniform vec3        u_start_seed;
uniform vec3        u_scale;
uniform float       u_amp;
uniform float       u_rnd_rotate;
uniform vec2        u_voro;
uniform vec3        u_mask;
uniform int         u_max_steps;


// random stepper
vec3 rseed;
float rnd1() { rseed += 1.01; return hash1(rseed); }
vec3 rnd3() { rseed += 1.01; return hash3(rseed); }


// one layer of noise
vec4 NOISE(in vec3 p, in float level)
{
    p = mod(p, 1.)+.5;

#if RANDOM_ROTATE == 1
    p = rotateAxis(p, rnd3(), rnd1() * u_rnd_rotate);
    //p = rotateAxis(p, vec3(1,1.7,2.7), 2.74);
#endif
    p = p * (u_scale.x + u_scale.y * level);

    p += rnd3();

#if USE_ALPHA == 1
            vec4 col =
    #if NOISE_FUNC == 0
                    vec4(hash3(floor(p)), hash1(floor(p+3.33)));
    #elif NOISE_FUNC == 1
                    vec4(noise3(p), noise1(p+23.+rnd3()*100.));
    #elif NOISE_FUNC == 2
                    vec4(vnoise3(p, u_voro.x, u_voro.y),
                             vnoise1(p+23.+rnd3()*100., u_voro.x, u_voro.y));
    #endif
#else
            vec4 col = vec4(
    #if NOISE_FUNC == 0
                    hash3(floor(p))
    #elif NOISE_FUNC == 1
                    noise3(p)
    #elif NOISE_FUNC == 2
                    vnoise3(p, u_voro.x, u_voro.y)
    #endif
            , 1.);
#endif

#if MASK == 0
    return col;
#elif MASK == 1
    return col * smoothstep(u_mask.x, u_mask.y,
                                            abs(length(col.xyz)/sqrt(3.) - u_mask.z));
#endif
}


// one pixel of noise mixture
vec4 mainFunc(in vec2 uv)
{
    rseed = u_start_seed;

    // fractal noise
#if FRACTAL_MODE == 0
    return NOISE(vec3(uv, 0.), 0.);

    // weighted average
#elif FRACTAL_MODE == 1

    vec4 col = vec4(0.);

    float a = 1., lvl = 0., sum = 0.;
    int i=0;
    while (a > 0.01 && i++ < u_max_steps)
    {
    col += a * NOISE(vec3(uv, 0.), lvl);
            sum += a;
            lvl += 1.;
            a *= u_amp;
    }
    return col / sum;

    // maximum
#elif FRACTAL_MODE == 2

    vec4 col = vec4(0.,0.,0.,0.);

    float a = 1., lvl = 0.;
    int i=0;
    while (a > 0.01 && i++ < u_max_steps)
    {
    col = max(col, a * NOISE(vec3(uv, 0.), lvl));
            lvl += 1.;
            a *= u_amp;
    }
    return col;

    // recursive
#elif FRACTAL_MODE == 3

    vec4 col = vec4(0.);

    for (int i = 0; i < u_max_steps; ++i)
        col = NOISE(vec3(uv, 0.) + col.xyz, float(i));

    return col;

#endif

}

void main()
{
    vec4 col = mainFunc(v_texCoord);

    // wrapping doesnt work yet
#if 0
    vec4 colx = mainFunc(v_texCoord + vec2(1.,0.));
    vec4 coly = mainFunc(v_texCoord + vec2(0.,1.));
    float fx = smoothstep(0.2, 0., v_texCoord.x),
              fy = smoothstep(0.2, 0., v_texCoord.y);
    col = mix(col, colx, fx);
    col = mix(col, coly, fy);
#endif

#if USE_ALPHA == 0
    col.a = 1.;
#endif

    // ---- post-proc ----

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
