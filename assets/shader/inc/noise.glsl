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

/** One value and three derivatives for three dimensions
    http://www.iquilezles.org/www/articles/morenoise/morenoise.htm */
vec4 noise1d(in vec3 x)
{
    vec3 i = floor(x);
    vec3 u = fract(x);

    vec3 d0 = 30.*u*u*(u*(u-2.)+1.);

    u = NOISE_TRANSITION(u);

    float   a = hash1(i + vec3(0.,0.,0.)),
            b = hash1(i + vec3(1.,0.,0.)),
            c = hash1(i + vec3(0.,1.,0.)),
            d = hash1(i + vec3(1.,1.,0.)),
            e = hash1(i + vec3(0.,0.,1.)),
            f = hash1(i + vec3(1.,0.,1.)),
            g = hash1(i + vec3(0.,1.,1.)),
            h = hash1(i + vec3(1.,1.,1.)),

            k0 =   a,
            k1 =   b - a,
            k2 =   c - a,
            k3 =   e - a,
            k4 =   a - b - c + d,
            k5 =   a - c - e + g,
            k6 =   a - b - e + f,
            k7 = - a + b + c - d + e - f - g + h;

    return vec4(
            k0 + k1*u.x + k2*u.y + k3*u.z
               + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z,
            d0.x * (k1 + k4*u.y + k6*u.z + k7*u.y*u.z),
            d0.y * (k2 + k5*u.z + k4*u.x + k7*u.z*u.x),
            d0.z * (k3 + k6*u.x + k5*u.y + k7*u.x*u.y) );
}
















// --- signed [-1,1] versions ---

/** One noise value from one dimension */
float snoise1(float x) { return noise1(x) * 2. - 1.; }

/** Two noise values from one dimension */
vec2 snoise2(float x) { return noise2(x) * 2. - 1.; }

/** Three noise values from one dimension */
vec3 snoise3(float x) { return noise3(x) * 2. - 1.; }

/** One noise value from two dimensions */
float snoise1(in vec2 x) { return noise1(x) * 2. - 1.; }

/** Two noise values from two dimensions */
vec2 snoise2(in vec2 x) { return noise2(x) * 2. - 1.; }

/** Three noise values from two dimensions */
vec3 snoise3(in vec2 x) { return noise3(x) * 2. - 1.; }

/** One noise value from three dimensions */
float snoise1(in vec3 x) { return noise1(x) * 2. - 1.; }

/** Two noise values from three dimensions */
vec2 snoise2(in vec3 x) { return noise2(x) * 2. - 1.; }

/** Three noise values from three dimensions */
vec3 snoise3(in vec3 x) { return noise3(x) * 2. - 1.; }

/** One value and three derivatives for three dimensions
    http://www.iquilezles.org/www/articles/morenoise/morenoise.htm */
vec4 snoise1d(in vec3 x) { return noise1d(x) * 2. - 1.; }
