#include <to/header>

//#define AA 0-... (quadratic)
//#define SINE_OUT 0,1
//#define NUM_ITER 1-x
// 0 final, 1 average, 2 max, 3 min
//#define COL_MODE 3
//#define NUM_DIM 2,3,4
//#define MONOCHROME 0,1
//#define CALC_MODE basic,userfunc,evo

uniform vec4    u_kali_param;
uniform vec4    u_offset;
uniform vec2    u_scale;
uniform vec2    u_bright; // x=brightness, y=exponent
uniform float   u_freq;

//%mo_user_uniforms%

#if NUM_DIM == 2
    #define VEC vec2
    vec3 toVec3(in vec2 v) { return vec3(v, 0.); }
#elif NUM_DIM == 3
    #define VEC vec3
    vec3 toVec3(in vec3 v) { return v; }
#elif NUM_DIM == 4
    #define VEC vec4
    vec3 toVec3(in vec4 v) { return v.xyz; }
#endif

//%kali_user_param%
//%KaliSet%

vec3 kali_color(in vec2 pos)
{
#if CALC_MODE == 2
    vec3 r = evolvedKaliSet(pos);
#else
    #if NUM_DIM == 2
        vec2 p = pos * u_scale + u_offset.xy;
    #elif NUM_DIM == 3
        vec3 p = vec3(pos * u_scale, 0.) + u_offset.xyz;
    #elif NUM_DIM == 4
        vec4 p = vec4(pos * u_scale, 0., 0.) + u_offset;
    #endif


    #if COL_MODE == 3
        VEC av = VEC(1.);
    #else
        VEC av = VEC(0.);
    #endif

        for (int i=0; i<NUM_ITER; ++i)
        {
            p = abs(p) / dot(p, p);

    #if COL_MODE == 1
            av += p;
    #elif COL_MODE == 2
            av = max(av, p);
    #elif COL_MODE == 3
            av = min(av, p);
    #endif

            if (i < NUM_ITER - 1)
    #if CALC_MODE == 0
                p -= VEC(u_kali_param);
    #elif CALC_MODE == 1
                p -= VEC(kali_user_param(pos, p, i));
    #endif
        }

    #if COL_MODE == 0
        vec3 r = toVec3(p);
    #elif COL_MODE == 1
        vec3 r = toVec3(av) / float(NUM_ITER);
    #elif COL_MODE == 2 || COL_MODE == 3
        vec3 r = toVec3(av);
    #endif

#endif

#if SINE_OUT
    r *= u_freq * 3.14159265 * 2.;
    r = .5 + .5 * vec3(sin(r.x), sin(r.y), sin(r.z));
#endif

    return r;
}


void main()
{
#if AA == 0
    vec3 col = kali_color(v_texCoord.xy * 2. - 1.);
#else
    vec3 col = vec3(0.);
    for (int j=0; j<AA; ++j)
    for (int i=0; i<AA; ++i)
    {
        col += kali_color((v_texCoord.xy + vec2(i,j) / float(AA) * u_resolution.zw) * 2. - 1.);
    }
    col /= (AA * AA);
#endif
    col = pow(max(col * u_bright.x, vec3(0.)), vec3(u_bright.y));

#if MONOCHROME
    col = vec3(dot(col, vec3(.3,.6,.1)));
#endif

    fragColor = vec4(col, 1.);
}
