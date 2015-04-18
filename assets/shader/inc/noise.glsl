// needs hash functions included before

#ifndef NOISE_TRANSITION
    //#define NOISE_TRANSITION(x) ( x )
    //#define NOISE_TRANSITION(x) ( x*x*(3.-2.*x) )
    #define NOISE_TRANSITION(x) ( x*x*x*(x*(6.*x-15.)+10.) )
    //#define NOISE_TRANSITION(x) ( .5-.5*cos(x*3.14159265) )
#endif

// ---------------------- 1d noise ------------------------

/** One noise value from one dimension */
float noise1(float x)
{
    float i = floor(x);
    float f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(hash1(i), hash1(i + 1.), f);
}

/** Two noise values from one dimension */
vec2 noise2(float x)
{
    float i = floor(x);
    float f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(hash2(i), hash2(i + 1.), f);
}

/** Three noise values from one dimension */
vec3 noise3(float x)
{
    float i = floor(x);
    float f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(hash3(i), hash3(i + 1.), f);
}



// ---------------------- 2d noise ------------------------

/** One noise value from two dimensions */
float noise1(in vec2 x)
{
    vec2 i = floor(x);
    vec2 f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(mix( hash1(i),               hash1(i + vec2(1.,0.)),f.x),
               mix( hash1(i + vec2(0.,1.)), hash1(i + vec2(1.,1.)),f.x),f.y);
}

/** Two noise values from two dimensions */
vec2 noise2(in vec2 x)
{
    vec2 i = floor(x);
    vec2 f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(mix( hash2(i),               hash2(i + vec2(1.,0.)),f.x),
               mix( hash2(i + vec2(0.,1.)), hash2(i + vec2(1.,1.)),f.x),f.y);
}

/** Three noise values from two dimensions */
vec3 noise3(in vec2 x)
{
    vec2 i = floor(x);
    vec2 f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(mix( hash3(i),               hash3(i + vec2(1.,0.)),f.x),
               mix( hash3(i + vec2(0.,1.)), hash3(i + vec2(1.,1.)),f.x),f.y);
}



// ---------------------- 3d noise ------------------------

/** One noise value from three dimensions */
float noise1(in vec3 x)
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(
               mix(mix(hash1(i + vec3(0.,0.,0.)), hash1(i + vec3(1.,0.,0.)),f.x),
                   mix(hash1(i + vec3(0.,1.,0.)), hash1(i + vec3(1.,1.,0.)),f.x),
                   f.y),
               mix(mix(hash1(i + vec3(0.,0.,1.)), hash1(i + vec3(1.,0.,1.)),f.x),
                   mix(hash1(i + vec3(0.,1.,1.)), hash1(i + vec3(1.,1.,1.)),f.x),
                   f.y),
               f.z);
}

/** Two noise values from three dimensions */
vec2 noise2(in vec3 x)
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(
               mix(mix(hash2(i + vec3(0.,0.,0.)), hash2(i + vec3(1.,0.,0.)),f.x),
                   mix(hash2(i + vec3(0.,1.,0.)), hash2(i + vec3(1.,1.,0.)),f.x),
                   f.y),
               mix(mix(hash2(i + vec3(0.,0.,1.)), hash2(i + vec3(1.,0.,1.)),f.x),
                   mix(hash2(i + vec3(0.,1.,1.)), hash2(i + vec3(1.,1.,1.)),f.x),
                   f.y),
               f.z);
}

/** Three noise values from three dimensions */
vec3 noise3(in vec3 x)
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = NOISE_TRANSITION(f);

    return mix(
               mix(mix(hash3(i + vec3(0.,0.,0.)), hash3(i + vec3(1.,0.,0.)),f.x),
                   mix(hash3(i + vec3(0.,1.,0.)), hash3(i + vec3(1.,1.,0.)),f.x),
                   f.y),
               mix(mix(hash3(i + vec3(0.,0.,1.)), hash3(i + vec3(1.,0.,1.)),f.x),
                   mix(hash3(i + vec3(0.,1.,1.)), hash3(i + vec3(1.,1.,1.)),f.x),
                   f.y),
               f.z);
}
