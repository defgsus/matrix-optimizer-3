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


// ---- mode and dimension ----

#define M_BYPASS 0
#define M_ERROR 1
#define M_FPROP 2
#define M_BPROP 3
#define M_WEIGHT_INIT 4

#define A_LINEAR 0
#define A_TANH 1
#define A_LOGISTIC 2

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

// --------------- activation ------------------

#if ACTIVATION == A_LINEAR
    float activation_func(in float x) { return x; }
    float derivative_func(in float e, in float x) { return e; }
#elif ACTIVATION == A_TANH
    float activation_func(in float x) { return tanh(x); }
    float derivative_func(in float e, in float x) { return e * (1. - x * x); }
#elif ACTIVATION == A_LOGISTIC
    float activation_func(in float x) { return 1. / (1. + exp(-x)); }
    float derivative_func(in float e, in float x) { return e * x * (1. - x); }

#endif

// ------ activation overloads for multi-dimensional types --------

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


// ------------ inputs from textures ---------------

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
TYPE prev_output_state(in int i)
{
    ivec2 pix = ivec2(i % OUTPUT_RES.x, i / OUTPUT_RES.x);
    TYPE x = texelFetch(u_tex_prev_out, pix, 0).TYPE_SWIZZLE;
#if !SIGNED_INPUT_PREV
    x = x * 2. - 1.;
#endif
    return x;
}
#endif



#if defined(ERROR_RES) && (LABEL_INPUT == 0)
TYPE error_state(in int i)
{
    ivec2 pix = ivec2(i % ERROR_RES.x, i / ERROR_RES.x);
    TYPE x = texelFetch(u_tex_error, pix, 0).TYPE_SWIZZLE;
#if !SIGNED_INPUT_ERROR
    x = x * 2. - 1.;
#endif
    return x;
}
#endif

#if defined(ERROR_RES) && (LABEL_INPUT == 1)
TYPE label_state(in int i)
{
    ivec2 pix = ivec2(i % ERROR_RES.x, i / ERROR_RES.x);
    TYPE x = texelFetch(u_tex_error, pix, 0).TYPE_SWIZZLE;
#if !SIGNED_INPUT_ERROR
    x = x * 2. - 1.;
#endif
    return x;
}
#endif

#ifdef WEIGHT_RES
TYPE weight(in int i, in int j)
{
    int w = j * NUM_IN + i;
    ivec2 pix = ivec2(w % WEIGHT_RES.x, w / WEIGHT_RES.x);
    TYPE x = texelFetch(u_tex_weight, pix, 0).TYPE_SWIZZLE;
#if !SIGNED_INPUT_WEIGHT
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
// prop error derivative -> weight,input for output cell o
TYPE bprop_weight(in int i, in int o, in TYPE err_der)
{
    TYPE inp = input_state(i);
    TYPE w = weight(i, o);
    return w + u_learnrate * inp * err_der;
}
#endif




void main()
{
    // pixel coordinate in output texture
    ivec2 pix = ivec2(v_pixelCoord);

#if MODE == M_BYPASS
    int i = pix.y * OUTPUT_RES.x + pix.x;
    TYPE val = input_state(i);
    const int signed = SIGNED_OUTPUT;

#elif MODE == M_ERROR
    int i = pix.y * ERROR_RES.x + pix.x;
    TYPE inp = input_state(i);
    TYPE err = label_state(i) - inp;
    TYPE val = derivative_func(err, inp);
    const int signed = SIGNED_OUTPUT_ERROR;

#elif MODE == M_FPROP
    int o = pix.y * OUTPUT_RES.x + pix.x;
    TYPE val = fprop(o);
    const int signed = SIGNED_OUTPUT;

#elif MODE == M_BPROP
    int x = pix.y * WEIGHT_RES.x + pix.x; 	// weight index
    int i = x % NUM_IN; 			// input cell
    int o = x / NUM_IN; 			// output cell
    TYPE err = error_state(o);			// error at output
    TYPE val = bprop_weight(i, o, err);
    const int signed = SIGNED_OUTPUT_WEIGHT;

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
    const int signed = SIGNED_OUTPUT_WEIGHT;

#endif


    if (signed != 0)
        fragColor = TYPE_TO_4(val);
    else
        fragColor = clamp(TYPE_TO_4(val) * .5 + .5, 0., 1.);

#if CLAMP_ALPHA_1
    fragColor.a = 1.;
#endif
}

