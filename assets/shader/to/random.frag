#include <to/header>
#include <constants>
#include <rotate>
#include <color>
#include <dh/hash>
#include <noise>
#include <iq/vnoise>

//define RANDOM_ROTATE 0,1
//define NOISE_FUNC on of NF_... below
//define MASK 0,1
//define USE_ALPHA 0,1
//define FRACTAL_MODE one of FMODE_.. below
//define NUM_TURB_STEPS

#define NF_RECT 0
#define NF_PERLIN 1
#define NF_VORONOISE 2
#define NF_CIRCLE 3

#define FMODE_SINGLE 0
#define FMODE_AVERAGE 1
#define FMODE_MAX 2
#define FMODE_RECURSIVE 3
#define FMODE_RANDOM 4

uniform vec4        u_color;
uniform vec3        u_hsv;
uniform vec3        u_gamma_exp;
uniform vec3        u_start_seed;
uniform vec3        u_scale;
uniform float       u_amp;
uniform float       u_rnd_rotate;
uniform float       u_voro;
uniform float       u_smooth;
uniform vec3        u_mask;
uniform int         u_max_steps;
uniform vec4        u_recursive; // xyz=dot, w=amt
uniform vec3        u_shape; // rad, rndRad, rndPos
uniform vec3        u_turb;

// random stepper
vec3 rseed;
float rnd1() { rseed += 1.01; return hash1(rseed); }
vec3 rnd3() { rseed += 1.01; return hash3(rseed); }


#if NOISE_FUNC == NF_CIRCLE
// random circles, based on iq's voronoise
vec4 random_circles4(in vec3 x)
{
    float smoot = u_smooth;
    float rad = u_shape.x;
    float rndRad = u_shape.y;
    float rndPos = u_shape.z;

    vec3 p = floor(x);
    vec3 f = fract(x);

    vec4 va = vec4(0.0);
    float wt = smoot;
    for( int j=-3; j<=2; j++ )
    for( int i=-3; i<=2; i++ )
    {
        vec3  g = vec3(float(i), float(j), 0.);
        vec3  xyr = hash3(p + g);
        vec2  r = g.xy - f.xy + xyr.xy * rndPos * 2.;
        float w = length(r) - rad * (1. + rndRad*(xyr.r-1.));
        w = smoothstep(smoot+0.01, .0, w);
        va += w*vec4(hash3((p + g)*1.11 + 3.), hash1((p + g)*1.131 + 5.) );
        wt += w;
    }

    return clamp(va/wt * (1.+.3*smoot), 0., 1.);
}
#endif

// one layer of noise
vec4 NOISE(in vec3 p, in float level)
{
    //p = mod(p, 1.)+.5;

#if RANDOM_ROTATE == 1
    p.xy = rotateAxis(p, rnd3(), rnd1() * u_rnd_rotate).xy;
#endif
    p = p * (u_scale.x + u_scale.y * level);

    // random offset per layer
    p += rnd3() * 5.;

#if USE_ALPHA == 1
            vec4 col =
    #if NOISE_FUNC == NF_RECT
                    vec4(hash3(floor(p)), hash1(floor(p+3.33)))
    #elif NOISE_FUNC == NF_PERLIN
                    vec4(noise3(p), noise1(p+23.+rnd3()*100.))
    #elif NOISE_FUNC == NF_VORONOISE
                    vec4(vnoise3(p, u_voro, u_smooth),
                             vnoise1(p+23.+rnd3()*100., u_voro, u_smooth))
    #elif NOISE_FUNC == NF_CIRCLE
                random_circles4(p)
    #endif
                ;
#else
            vec4 col = vec4(
    #if NOISE_FUNC == NF_RECT
                    hash3(floor(p))
    #elif NOISE_FUNC == NF_PERLIN
                    noise3(p)
    #elif NOISE_FUNC == NF_VORONOISE
                    vnoise3(p, u_voro, u_smooth)
    #elif NOISE_FUNC == NF_CIRCLE
                    random_circles4(p).xyz
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

vec2 turbulence(in vec2 p, in float lambda, in float omega)
{
    float l = lambda, o = omega;
    for (int i=0; i<NUM_TURB_STEPS; ++i)
    {
        if (l<0.0001)
            break;
        p += l * (-1.+2.*noise2(vec3(o * p, 3.5)));
        l *= lambda;
        o *= omega;
    }
    return p;
}

// one pixel of noise mixture
vec4 mainFunc(in vec2 uv)
{
    rseed = u_start_seed;

    // turbulence
    uv = turbulence(uv, u_turb.x, u_turb.y * u_scale.x);

    // single noise layer
#if FRACTAL_MODE == FMODE_SINGLE
    return NOISE(vec3(uv, 0.), 0.);

    // weighted average
#elif FRACTAL_MODE == FMODE_AVERAGE

    vec4 col = vec4(0.);

    float a = 1., lvl = 0., sum = 0.0001;
    int i=0;
    while (a > 0.01 && i++ < u_max_steps)
    {
        col += a * NOISE(vec3(uv, float(lvl)), lvl);
        sum += a;
        lvl += 1.;
        a *= u_amp;
    }
    return col / sum;

    // weighted maximum
#elif FRACTAL_MODE == FMODE_MAX

    vec4 col = vec4(0.,0.,0.,0.);

    float a = 1., lvl = 0., sum = 1.;
    int i=0;
    while (a > 0.01 && i++ < u_max_steps)
    {
        col = max(col, a * NOISE(vec3(uv, float(lvl)), lvl));
        lvl += 1.;
        sum = max(sum, a);
        a *= u_amp;
    }
    return col / sum;

    // recursive
#elif FRACTAL_MODE == FMODE_RECURSIVE
    vec4 col = vec4(0.);

    for (int i = 0; i < u_max_steps; ++i)
        col = NOISE(vec3(uv, float(i))
                    + u_recursive.w * dot(u_recursive.xyz, col.xyz),
                    float(i));

    return col;

    // random recursive
#elif FRACTAL_MODE == FMODE_RANDOM
    // XXX NOT MAKING SENSE YET
    vec3 pos = vec3(uv, 0.) + u_start_seed;
    for (int i = 0; i < u_max_steps; ++i)
        pos = u_recursive.w * (u_recursive.xyz, NOISE(pos, float(i)).xyz);
    pos += vec3(uv, 0.);

    return NOISE(pos, float(u_max_steps-1));

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
