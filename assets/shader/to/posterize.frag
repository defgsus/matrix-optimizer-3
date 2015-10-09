#include <to/header>

uniform sampler2D   u_tex;
uniform vec2        u_quant; // x=pos, y=color
uniform float       u_tolerance; // for smoothstep

//#define QUANT_COLOR 0,1
//#define QUANT_TEX 0,1
//#define MONOCHROME 0,1
//#define SHAPE 0-5
//#define SMOOTH_STEP 0,1

float Step(in float x, in float y)
{
#if SMOOTH_STEP
    return smoothstep(x-u_tolerance, x+u_tolerance, y);
#else
    return step(x, y);
#endif
}

float dither_func(in float val, in vec2 pos)
{
    vec2 m = abs(mod(pos, 1.)-.5)*2.;
#if SHAPE == 0
    // h-line
    return Step(m.y, val);
#elif SHAPE == 1
    // v-line
    return Step(m.x, val);
#elif SHAPE == 2
    // cross
    return Step(min(m.x,m.y), val);
#elif SHAPE == 3
    // circle
    return Step(length(m), val);
#elif SHAPE == 4
    // diamond
    return Step((m.x+m.y)*.71, val);
#elif SHAPE == 5
    // square
    return Step(max(m.x,m.y), val);
#endif
}

// pos in in pixels!
vec3 posterize(in sampler2D tex, in vec2 pos)
{
    // texture lookup pos
#if QUANT_TEX
    vec2 texuv = floor(pos / u_resolution.y * u_quant.x) / u_quant.x
            * vec2(u_resolution.y/u_resolution.x, 1.);
#else
    vec2 texuv = pos / u_resolution.xy;
#endif

    // shape-grid pos
    vec2 griduv = pos / u_resolution.y * u_quant.x;

    // input color
    vec3 color = texture(tex, texuv).xyz;
#if QUANT_COLOR
    color = floor(color*u_quant.y)/u_quant.y;
#endif

#if MONOCHROME
    return vec3(dither_func(dot(color,vec3(.3,.6,.1)), griduv));
#else
    return vec3(dither_func(color.x, griduv),
                dither_func(color.y, griduv),
                dither_func(color.z, griduv));
#endif
}

void main()
{
    vec3 col = posterize(u_tex, v_texCoord * u_resolution.xy);

    fragColor = vec4(col, 1.);
}
