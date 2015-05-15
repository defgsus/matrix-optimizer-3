#include <to/header>
#include <constants>
#include <color>

// quadratic, exponential, user
//define LENS_MODE 0,1,2
//define CHROMATIC 0,1

uniform sampler2D       u_tex;
uniform vec3 		u_lens;         // def, per-spec, size
uniform vec2		u_lens_set;     // exp, normal-radius
uniform vec2		u_lens_center;
uniform vec3		u_spec;         // shift1, shift2, saturation
uniform int             u_num_samples;


vec3 spectral(in float f)
{
    // edge fade
    float v = smoothstep(-0.03, 0.2, f);
    v = min(v, smoothstep(1.03, 0.8, f));
    // shift middle part together
    f = f * f * (3. - 2. * f);
    // scale
    f = max(0., f*(0.9+u_spec.x) - 0.08 + u_spec.y);
    return hsv2rgb(vec3(f, u_spec.z, v));
}

float lengthx(in vec2 x, in float e)
{
    return pow( pow(abs(x.x),e) + pow(abs(x.y),e), 1./e );
}

//%user_code%


// Insert [-1,1] receive [0,1]
vec2 lens_lookup(in vec2 uv, float f)
{
    uv -= u_lens_center;

#if LENS_MODE == 0
    uv += f * uv * dot(uv, uv);
#elif LENS_MODE == 1
    uv += f * uv * lengthx(uv, u_lens_set.x);
#elif LENS_MODE == 2
    uv = lens_distortion(uv, f);
#endif

    // 'normalize'
    uv /= 1. + u_lens_set.y * f;

    uv += u_lens_center;

    return u_lens.z * uv * .5 + .5;
}


vec4 lens_chroma(in vec2 uv)
{
    float f = u_lens.x, f1 = u_lens.y;
    vec4 col = vec4(0.);
    float sum = 0.;

    for (int i=0; i<u_num_samples; ++i)
    {
        float hue = (float(i)+.5) / u_num_samples;
        // spectral color
        vec4 sp = vec4(pow(spectral(hue), vec3(1.1)), 1.);
        col += sp * texture(u_tex, lens_lookup(uv, f+hue*f1));
        sum += .35;
    }
    return col / sum;
}

void main()
{
#if CHROMATIC == 0
    vec4 col = texture(u_tex, lens_lookup(v_texCoord*2.-1., u_lens.x));
#else
    vec4 col = lens_chroma(v_texCoord*2.-1.);
#endif

    fragColor = col;
}
