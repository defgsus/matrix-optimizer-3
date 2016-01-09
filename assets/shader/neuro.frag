#version 130

uniform sampler2D   u_tex_input;
uniform sampler2D   u_tex_weight;
uniform sampler2D   u_tex_error;
uniform sampler2D   u_tex_prev_out;
uniform float       u_learnrate;
uniform int         u_rseed;
uniform vec4        u_weight_init;

in vec2 v_pixelCoord;

out vec4 fragColor;

// ----

#define M_BYPASS 0
#define M_ERROR 1
#define M_FPROP 2
#define M_BPROP 3
#define M_WEIGHT_INIT 4

#if TYPE_DIMENSION == 1
#   define TYPE float
#   define TYPE_SWIZZLE x
#   define TYPE_TO_4(x) vec4(x, 0., 0., 0.)
#   define TYPE_HASH(x) hash1(x)
#elif TYPE_DIMENSION == 2
#   define TYPE vec2
#   define TYPE_SWIZZLE xy
#   define TYPE_TO_4(x) vec4(x, 0., 0.)
#   define TYPE_HASH(x) hash2(x)
#elif TYPE_DIMENSION == 3
#   define TYPE vec3
#   define TYPE_SWIZZLE xyz
#   define TYPE_TO_4(x) vec4(x, 0.)
#   define TYPE_HASH(x) hash3(x)
#elif TYPE_DIMENSION == 4
#   define TYPE vec4
#   define TYPE_SWIZZLE xyzw
#   define TYPE_TO_4(x) x
#   define TYPE_HASH(x) vec4(hash3(x), hash1((x)*1.1618005))
#endif

#if MODE == M_WEIGHT_INIT
    #include <dh/hash>
    // random number between -1, 1
    TYPE random(int seedx, int seedy)
        { return TYPE_HASH(vec2(float(seedx), float(seedy))) * 2. - 1.; }
#endif


float activation_func(in float x) { return tanh(x); }
float derivative_func(in float e, in float x) { return e * (1. - x * x); }

vec2 activation_func(in vec2 x)
        { return vec2(activation_func(x.x), activation_func(x.y)); }
vec3 activation_func(in vec3 x)
        { return vec3(activation_func(x.x), activation_func(x.y),
                                  activation_func(x.z)); }
vec4 activation_func(in vec4 x)
        { return vec4(activation_func(x.x), activation_func(x.y),
                                  activation_func(x.z), activation_func(x.w)); }

vec2 derivative_func(in vec2 e, in vec2 x)
        { return vec2(derivative_func(e.x, x.x), derivative_func(e.y, x.y)); }
vec3 derivative_func(in vec3 e, in vec3 x)
        { return vec3(derivative_func(e.x, x.x), derivative_func(e.y, x.y),
                                  derivative_func(e.z, x.z)); }
vec4 derivative_func(in vec4 e, in vec4 x)
        { return vec4(derivative_func(e.x, x.x), derivative_func(e.y, x.y),
                                  derivative_func(e.z, x.z), derivative_func(e.w, x.w)); }

#ifdef INPUT_RES
// input state for cell i
TYPE input_state(in int i)
{
    ivec2 pix = ivec2(i % INPUT_RES.x, i / INPUT_RES.x);
    TYPE x = texelFetch(u_tex_input, pix, 0).TYPE_SWIZZLE;
#if !SIGNED_INPUT
    x = x * 2. - 1.;
#endif
    return x;
}
#endif

#ifdef OUTPUT_RES
// previous output state
TYPE output_state(in int i)
{
    ivec2 pix = ivec2(i % OUTPUT_RES.x, i / OUTPUT_RES.x);
    TYPE x = texelFetch(u_tex_prev_out, pix, 0).TYPE_SWIZZLE;
#if !SIGNED_INPUT
    x = x * 2. - 1.;
#endif
    return x;
}
#endif

#if defined(ERROR_RES) && defined(OUTPUT_RES)
TYPE error_state(in int i)
{
    ivec2 pix = ivec2(i % ERROR_RES.x, i / ERROR_RES.x);
    TYPE inp = texelFetch(u_tex_error, pix, 0).TYPE_SWIZZLE;
#if LABEL_INPUT
    inp -= output_state(i);
#endif
    return inp;
}
#endif

#if defined(ERROR_RES) && (LABEL_INPUT)
TYPE label_state(in int i)
{
    ivec2 pix = ivec2(i % ERROR_RES.x, i / ERROR_RES.x);
    TYPE inp = texelFetch(u_tex_error, pix, 0).TYPE_SWIZZLE;
    return inp;
}
#endif

#ifdef WEIGHT_RES
TYPE weight(in int i, in int j)
{
    int w = j * NUM_IN + i;
    ivec2 pix = ivec2(w % WEIGHT_RES.x, w / WEIGHT_RES.x);
    TYPE x = texelFetch(u_tex_weight, pix, 0).TYPE_SWIZZLE;
#if !SIGNED_WEIGHTS
    x = x * 2. - 1.;
#endif
    return x;
}
#endif

#if MODE == M_FPROP
// prop input->weights for output cell o
TYPE fprop(in int o)
{
    TYPE sum = TYPE(0.);
    for (int i = 0; i < NUM_IN; ++i)
    {
        sum += input_state(i) * weight(i, o);
    }
    return activation_func(sum);
}
#endif

#if MODE == M_BPROP
// prop error->weight,input for output cell o
TYPE bprop_weight(in int i, in int o, in TYPE e)
{
    TYPE inp = input_state(i);
    TYPE w = weight(i, o);
    TYPE d = derivative_func(e, output_state(o));
    return w + u_learnrate * inp * d;
}
#endif

// fragCoord is in pixels
void main()
{
    ivec2 pix = ivec2(v_pixelCoord);


#if MODE == M_BYPASS
    int i = pix.y * OUTPUT_RES.x + pix.x;
    TYPE val = input_state(i);

#elif MODE == M_ERROR
    int i = pix.y * OUTPUT_RES.x + pix.x;
    TYPE val = input_state(i) - label_state(i);

#elif MODE == M_FPROP
    int o = pix.y * OUTPUT_RES.x + pix.x;
    TYPE val = fprop(o);

#elif MODE == M_BPROP
    int x = pix.y * WEIGHT_RES.x + pix.x; 	// weight index
    int i = x % NUM_IN; 			// input cell
    int o = x / NUM_IN; 			// output cell
    TYPE err = error_state(o);			// error at output
    TYPE val = bprop_weight(i, o, err);

#elif MODE == M_WEIGHT_INIT
    int x = pix.y * WEIGHT_RES.x + pix.x; 	// weight index
    int i = x % NUM_IN; 			// input cell
    int o = x / NUM_IN; 			// output cell
    // respective input/output vectors [-1,1]
    vec2 iv = vec2(float(i % INPUT_RES.x), float(i / INPUT_RES.x))
                            / vec2(float(INPUT_RES.x), float(INPUT_RES.y));
    vec2 ov = vec2(float(o % OUTPUT_RES.x), float(o / OUTPUT_RES.x))
                            / vec2(float(OUTPUT_RES.x), float(OUTPUT_RES.y));
    // distance based receptive field
    float d = distance(iv, ov) / sqrt(2.);
          d = pow(max(0., 1. - d), u_weight_init.w);
    // noise
    TYPE n = random(i + u_rseed, o + u_rseed * NUM_IN)
                * u_weight_init.y + u_weight_init.x;
    TYPE val = n + u_weight_init.z * d;
#endif


#if SIGNED_OUTPUT
    fragColor = TYPE_TO_4(val);
#else
    fragColor = clamp(TYPE_TO_4(val) * .5 + .5, 0., 1.);
#endif

#if CLAMP_ALPHA_1
    fragColor.a = 1.;
#endif
}

