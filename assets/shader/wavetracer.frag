#version 330

// -- inputs from vertex shader --

in vec2 v_pos;                  // the pixel position, normalized to [-1,1]
in vec2 v_screen;               // the pixel position,
                                // true resolution [0, width/height)
// -- output to framebuffer --

out vec4 fragColor;             // color output

// -- inputs from program --

uniform vec4 u_resolution;      // width, height, 1./width, 1./height
uniform float u_time;           // scene time in seconds
uniform mat4 u_transformation;  // object's own transformation matrix



// --------------- user defined --------------------------

// 0 = true values
// 1 = readable colorization of true values with multi-sampling
// 2 = nice visual for modelling
// #define _RENDER_MODE 0
// uniform int     _MAX_TRACE_STEPS;
// uniform int     _MAX_REFLECT;
// uniform int     _NUM_SAMPLES;

uniform vec4 	_SNDSRC;                // xyz = pos, w = radius/thickness
uniform float   _FUDGE;                 // ray step precision
uniform float   _MIC_ANGLE;             // microphone opening angle
uniform float   _MAX_TRACE_DIST;
uniform float   _DAMPING;               // global reflection amount
uniform float   _BRIGHTNESS;            // amount of visual light/sound, visual rendermodes
uniform float   _EPSILON;
uniform int     _PASS_NUMBER;           // number of pass (for multi-sampling)
uniform vec3    _SND_COLOR;             // visual sound representation

//!mo_user_functions!
/* expects these functions

# include <iq/distfunc>

float DE_room(in vec3 p)
{
        float d = 1000.;

        //d = min(d, length(p-vec3(1,0,-2))-1.);

        //d = min(d, -sdTorus(p, vec2(10, 4)));
        d = min(d, -sdBox(p, vec3(15., 5., 15.)));

        p = mod(p, 3.) - 1.5;
        d = min(d, sdBox(p, vec3(0.4, 2.4, .4)));
        return d;
}

float DE_sound(in vec3 p)
{
        return length(p);
        return length(p.yz);
}

// return reflection coefficient [0,1]
float DE_reflection(in vec3 p, in vec3 n)
{
        return 1.;
        //return 0.5+0.5*smoothstep(0,0.1,length(p-vec3(1,0,-2))-1.4);
        //return smoothstep(0,2.9, abs(p.y));
}
*/

// -------------------- helper --------------------------

// Dave_Hoskins https://www.shadertoy.com/view/4djSRW (as of Jan 2015)
vec3 _hash3(vec3 p)
{
    p  = fract(p * vec3(5.3983, 5.4427, 6.9371));
    p += dot(p.yzx, p.xyz  + vec3(21.5351, 14.3137, 15.3219));
    return fract(vec3(p.x * p.z * 95.4337,
                                          p.x * p.y * 97.597,
                                          p.y * p.z * 93.8365));
}

vec3 _l2w(vec3 l, vec3 normal)
{
        vec3 binormal,tangent;
        if( abs(normal.x) > abs(normal.z) )
                binormal = normalize(vec3(-normal.y, normal.x, 0.));
        else
                binormal = normalize(vec3(0., -normal.z, normal.y));
        tangent = cross( binormal, normal );
        return l.x*tangent + l.y*binormal + l.z*normal;
}

/*struct Ray
{
        vec3 ro, rd, hit, n;
        float t, way, phase;
};*/


// -------------------- WAVE TRACER ---------------------

float _DE_sound(in vec3 p) { return DE_sound(p - _SNDSRC.xyz) - _SNDSRC.w; }
float _DE_reflection(in vec3 p, in vec3 n) { return DE_reflection(p, n) * _DAMPING; }

float _DE(in vec3 p)
{
    return min(DE_room(p), _DE_sound(p));
}

vec3 _DE_normal(in vec3 p, float eps = 0.0001)
{
    vec2 e = vec2(eps, 0.);
    return normalize(vec3(
                        _DE(p + e.xyy) - _DE(p - e.xyy),
                        _DE(p + e.yxy) - _DE(p - e.yxy),
                        _DE(p + e.yyx) - _DE(p - e.yyx) ));
}

vec3 _DE_room_normal(in vec3 p, float eps = 0.0001)
{
    vec2 e = vec2(eps, 0.);
    return normalize(vec3(
                        DE_room(p + e.xyy) - DE_room(p - e.xyy),
                        DE_room(p + e.yxy) - DE_room(p - e.yxy),
                        DE_room(p + e.yyx) - DE_room(p - e.yyx) ));
}

vec3 _DE_sound_normal(in vec3 p, float eps = 0.0001)
{
    vec2 e = vec2(eps, 0.);
    return normalize(vec3(
                        _DE_sound(p + e.xyy) - _DE_sound(p - e.xyy),
                        _DE_sound(p + e.yxy) - _DE_sound(p - e.yxy),
                        _DE_sound(p + e.yyx) - _DE_sound(p - e.yyx) ));
}


#if 1
float _DE_trace(in vec3 ro, in vec3 rd, in float len = 10., int steps = 100)
{
    float t = 0.;
    for (int i=0; i<steps && t < len; ++i)
    {
        vec3 p = ro + t * rd;
        float d = _DE(p);

        t += d * _FUDGE;
    }
    return t;
}
#else
// try to bend the ray (not working yet, needs to return it's position)
float _DE_trace(in vec3 ro, in vec3 rd, in float len = 10., int steps = 100)
{
        float t = 0.;
        vec3 p = ro;
        for (int i=0; i<steps && t < len; ++i)
        {
                float d = _DE(p);

                d *= _FUDGE;
                d = min(0.05, d);
                t += d;
                rd = normalize(rd - 0.03 * (1. + d) * _DE_room_normal(p, _EPSILON));
                p += d * rd;
        }
        return t;
}
#endif



// path-tracer returning the messured sound variables
// x = amplitude [0,1],
// y = true distance [0, MAX_REFLECT * MAX_TRACE_DIST]
// z = reflection count [0, MAX_REFLECT]
vec3 _cast(in vec3 ro, in vec3 rd, in vec3 seed)
{
    vec3 ret = vec3(0);
    float amp = 1.;

//	Ray ray = Ray(ro, rd, vec3(0.), vec3(0.), 0., 0., 1.);

    for (int i=0; i<_MAX_REFLECT+1; ++i)
    {
        // trace
        float t = _DE_trace(ro, rd, _MAX_TRACE_DIST, _MAX_TRACE_STEPS);
        vec3 hit = ro + t * rd;
        ret.y += t;

        // end of ray?
        if (_DE(hit) > 0.1)
                break;

        // hit sound?
        if (_DE_sound(hit) <= 0.00001)
        {
            ret.x = amp;// / (1. + 0.3 * ret.y);
            ret.z = float(i);
            break;
        }

        // reflect
        vec3 n = _DE_normal(hit, _EPSILON);
        ro = hit + 0.001 * n;
        // random new direction
        rd = _l2w(normalize(_hash3(seed)-.5), n);

        seed += 1.618;
        amp *= _DE_reflection(hit, n);
    }

    return ret;
}

// converts sound variable to color
vec3 _var2col(in vec3 var)
{
    float d = smoothstep(0.,30.,var.y);
    float x = var.z / float(_MAX_REFLECT);
    //return var.x * vec3(1.-d,d,0.);
    //return vec3(var.x);
    return var.x * clamp(vec3(d*x, 1.-d*x, d*(1.-x)), 0., 1.);
}

// visual sound variables, multisampled, rendermode 1
vec3 _multi_cast(in vec3 ro, in vec3 rd, in vec3 seed)
{
    vec3 col = vec3(0.);
    for (int i=0; i<_NUM_SAMPLES; ++i)
    {
        vec3 rd1 = rd;//normalize(rd + .06*normalize(_hash3(seed*1.37)-.5));
        col += _var2col(_cast(ro, rd1, seed));
        seed += 2.222;
    }
    return clamp(col / float(_NUM_SAMPLES) * _BRIGHTNESS * 10., 0., 1.);
}


// --------------- visual raymarcher ----------------

#if _RENDER_MODE >= 2

float _DE_ambient(in vec3 ro, in vec3 rd, int steps = 20)
{
    float t = 0.0003, ma = 0.;
    for (int i=0; i<steps; ++i)
    {
            float d = DE_room(ro + t * rd);
            ma = max(ma, d);
            if (d < 0.0001) break;
            t += d * _FUDGE;
    }
    return min(1., ma/4.);
}

float _DE_shadow(in vec3 ro, in vec3 rd, float maxt, float k = 8., int steps = 20)
{
    float t = 0.1, res = 1.0;
    for (int i=0; i<steps && t < maxt; ++i)
    {
        float h = DE_room(ro + rd * t);
        if( h<0.0000001 )
            return 0.0;
        res = min( res, k*h/t );
        t += h * _FUDGE;
    }
    return res;
}

float _dist_amt(in float dist)
{
    return .3+.7*smoothstep(_MAX_TRACE_DIST/2., 0., dist);
}

// simple ray-marching with phong and reflection
vec3 _simple_cast(in vec3 ro, in vec3 rd)
{
    vec3 col = vec3(0);
    float amp = 1., t_all = 0.;

    for (int i=0; i<_MAX_REFLECT; ++i)
    {
        // trace
        float t = _DE_trace(ro, rd, _MAX_TRACE_DIST, _MAX_TRACE_STEPS);
        vec3 hit = ro + t * rd;
        t_all += t;

        // end of ray?
        if (_DE(hit) > 0.1)
            break;

        // hit sound?
        float dsnd = _DE_sound(hit);
        if (dsnd <= 0.002)
        {
            col += amp * _SND_COLOR;
            break;
        }

        // -- color --
        vec3 n = _DE_normal(hit, _EPSILON);
        vec3 ln = -_DE_sound_normal(hit, _EPSILON);
        // object color from normal
        vec3 c = 0.13 + 0.1*n;
        // edges
        float edg = smoothstep(1, 1-_EPSILON,
                               dot(n, _DE_normal(hit, pow(_EPSILON, 0.36))));
        c *= 1. - 0.9 * edg;
        // phong
        c += amp * _SND_COLOR * pow(max(0., dot(n, ln)), 2.)
                         * _dist_amt(dsnd)
                         * _DE_shadow(hit, ln, dsnd, 8., _MAX_TRACE_STEPS)
                         * _BRIGHTNESS;

        col += amp * c;

        // -- reflect --
        ro = hit + 0.001 * n;
        rd = reflect(rd, n);

        amp *= _DE_reflection(hit, n);
    }

    col *= _dist_amt(t_all);

    return clamp(col, 0., 1.);
}

#endif



// ----------------------------- projection --------------------------------

// fulldome projection, input [-1,1]
vec3 _spherical(vec2 scr)
{
    float
        rad = length(scr),
        phi;

    if (rad == 0.0)
        phi = 0.0;
    else
    {	if (scr.x < 0.0)
            phi = 3.14159265 - asin(scr.y / rad);
        else
            phi = asin(scr.y / rad);
    }

    float
        theta = rad * _MIC_ANGLE * 3.14159265 / 360.,

        cx = cos(phi),
        cy = cos(theta),
        sx = sin(phi),
        sy = sin(theta);

    return vec3( cx * sy, sx * sy, -cy );
}


#if _RENDER_MODE == 3

// returns the visual 'field value' at the given position
// for rendermode 3
vec3 _field_color(in vec3 p)
{
    vec3 seed = vec3(v_pos, 1.59);
    vec3 col = vec3(0.);

    float dr = DE_room(p);

    float edg = smoothstep(0.07, 0., abs(dr));
    float ins = smoothstep(0.07, 0., dr);

    col += edg + 0.1 * ins;

    // --sample light/sound --
    if (ins < 0.001)
    {
        float dsnd = _DE_sound(p);
        vec3 ln = -_DE_sound_normal(p, _EPSILON);

        float sh = _DE_shadow(p, ln, dsnd, 8., _MAX_TRACE_STEPS);
        col.z += sh;

        // diffuse sampling
        sh = 0.;
        for (int i=0; i<_NUM_SAMPLES; ++i)
        {
            vec3 snd = _cast(p, normalize(_hash3(seed*3.35+i)-.5), (seed+i)*1.17+1.55);
            sh += snd.x;
        }
        sh /= float(_NUM_SAMPLES);

        col += sh * _SND_COLOR * _BRIGHTNESS * 10.;
    }

    return col;
}

#endif

void main()
{
    // top-down
#if _RENDER_MODE == 3

    vec3 pos = vec3(v_pos * 15., 0.);
    pos = (u_transformation * vec4(pos,1.)).xyz;

    fragColor = vec4(_field_color(pos), 1.);
    //fragColor = vec4(_field_color(vec3(v_pos*2., 0.)), 1.);
    return;

#else

    float rad = length(v_pos);
    // out of view
    if (rad > 1.)
    {
        fragColor = vec4(0,0,0,1);
        return;
    }

    vec3 pos = vec3(0., 0., 0.);
    pos.xy += .1*v_pos;
    vec3 dir = _spherical(v_pos);
                            //normalize(vec3(v_pos, -2.));
                            //vec3(0., 0., -1.);

    pos = (u_transformation * vec4(pos,1.)).xyz;
    dir = (mat3(u_transformation)) * dir;

    vec3 seed = vec3(v_pos, float(_PASS_NUMBER) * 1.33);

    #if _RENDER_MODE == 0
        fragColor = vec4(_cast(pos, dir, seed), 1.);
    #elif _RENDER_MODE == 1
        fragColor = vec4(_multi_cast(pos, dir, seed), 1.);
    #elif _RENDER_MODE == 2
        fragColor = vec4(_simple_cast(pos, dir), 1.);
    #endif

#endif
}
