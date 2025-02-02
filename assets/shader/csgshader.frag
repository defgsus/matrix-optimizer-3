#version 330

// -- inputs from vertex shader --

in vec2 v_pos;                  // the pixel position, normalized to [-1,1]
in vec2 v_screen;               // the pixel position,
                                // true resolution [0, width/height)
// -- output to framebuffer --

out vec4 fragColor;             // color output

// -- inputs from program --

uniform vec4  u_resolution;     // width, height, 1./width, 1./height
uniform float u_time;           // scene time in seconds
uniform mat4  u_vtmatrix;       // view transform
uniform vec2  u_max_trace_dist; // x = per ray / y = overall
uniform float u_fudge;          // ray precision step (<1.0)
uniform float u_epsilon;        // normal estimation distance
uniform float u_scale;          // slice scale in RENDER_MODE 0
uniform float u_frequency;      // slice distance color frequency

// -- expected defines --

// #define MAX_TRACE_STEPS      // maximum number of ray steps
// #define MAX_REFLECTIONS      // maximum number of reflections
// #define RENDER_MODE          // 0-flat, 1-solid

// -------------------- RAY MARCHER ---------------------

// dist func must be named 'DE_scene'
//%dist_func%
//float DE_scene(in vec3 pos) { return length(pos - vec3(0., 0., -2.))-1.; }

/** Returns normal for any point in space */
vec3 DE_normal(in vec3 p, float eps = 0.0001)
{
    vec2 e = vec2(eps, 0.);
    return normalize(vec3(
                        DE_scene(p + e.xyy) - DE_scene(p - e.xyy),
                        DE_scene(p + e.yxy) - DE_scene(p - e.yxy),
                        DE_scene(p + e.yyx) - DE_scene(p - e.yyx) ));
}

/** Returns ambient occlusion factor, starting at ro, tracing along rd */
float DE_ambient(in vec3 ro, in vec3 rd, int steps = 20)
{
    float t = 0.0003, ma = 0.;
    for (int i=0; i<steps; ++i)
    {
            float d = DE_scene(ro + t * rd);
            ma = max(ma, d);
            if (d < 0.0001) break;
            t += d * u_fudge;
    }
    return min(1., ma/4.);
}

/** Returns shadow factor, starting at ro, tracing along rd,
    maximally travelling maxt units and/or steps.
    k is smoothing factor */
float DE_shadow(in vec3 ro, in vec3 rd, float maxt, float k = 8., int steps = 20)
{
    float t = 0.1, res = 1.0;
    for (int i=0; i<steps && t < maxt; ++i)
    {
        float h = DE_scene(ro + rd * t);
        if( h<0.0000001 )
            return 0.0;
        res = min( res, k*h/t );
        t += h * u_fudge;
    }
    return res;
}

/** Traces from ro along rd and returns reached way-distance */
float DE_trace(in vec3 ro, in vec3 rd, in float len = 10., int steps = 100)
{
    float t = 0.;
    for (int i=0; i<steps && t < len; ++i)
    {
        vec3 p = ro + t * rd;
        float d = DE_scene(p);
        if (d < 0.001)
            return t;

        t += d * u_fudge;
    }
    return t;
}

float DE_brightness(in float dist)
{
    return smoothstep(u_max_trace_dist.y, 0., dist);
}


// simple ray-marching with phong lighting
vec3 simple_cast(in vec3 ro, in vec3 rd)
{
    // trace
    float t = DE_trace(ro, rd, u_max_trace_dist.x, MAX_TRACE_STEPS);
    vec3 hit = ro + t * rd;

    // -- color --

    vec3 n = DE_normal(hit, u_epsilon);
    // object color from normal
    vec3 col = 0.13 + 0.1*n;
    col *= smoothstep(0.01, 0., DE_scene(hit));
    // phong
    vec3 ln = normalize(vec3(1.));
    col += pow(max(0., dot(n, ln)), 2.);

    col *= DE_brightness(t);

    return clamp(col, 0., 1.);
}




void main()
{
#if RENDER_MODE == 0
    vec2 uv = v_pos * vec2(u_resolution.x/u_resolution.y, 1.) * u_scale;
    vec3 pos = (u_vtmatrix * vec4(uv, 0., 0.)).xyz
                - u_vtmatrix[3].xyz;

    vec3 col = vec3(0.);
    float sd = DE_scene(pos);

    // inside/outside color
    float c = //smoothstep(0., 1., abs(sd)) *
            (.5 + .4 * sin(sd*6.283185307 * u_frequency));
    if (sd >= 0.)
        col = vec3(0., c*.7, c*.3);
    else
        col = vec3(c*.7, c*.3, 0.);
    // edge
    col += smoothstep(0.01, 0., abs(sd));

    fragColor = vec4(clamp(col, 0., 1.), 1.);

#elif RENDER_MODE == 1
    vec2 uv = v_pos * vec2(u_resolution.x/u_resolution.y, 1.);
    vec3 ro = -u_vtmatrix[3].xyz;
    vec3 rd = normalize(vec3(uv, -2.));
    rd = (u_vtmatrix * vec4(rd, 0.)).xyz;

    fragColor = vec4(simple_cast(ro, rd), 1.);
#endif

}
