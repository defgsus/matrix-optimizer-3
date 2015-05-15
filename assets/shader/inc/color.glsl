#if 0
// https://www.shadertoy.com/view/MsS3Wc
vec3 hsv2rgb(in vec3 c)
{
    vec3 rgb = clamp( abs(mod(c.x * 6.0 + vec3(0.0,4.0,2.0), 6.0) - 3.0) -1.0, 0.0, 1.0 );
    rgb = rgb*rgb*(3.0-2.0*rgb);
    return c.z * mix(vec3(1.0), rgb, c.y);
}
#else
// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
#endif

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
#if 1
    vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);
    vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);
#else
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
#endif

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

/** Returns the approximation of the visible light color.
    f is in range [0,1] */
vec3 spectralColor(in float f)
{
    // fade in / out
    float v = smoothstep(-0.03, 0.2, f);
    v = min(v, smoothstep(1.03, 0.8, f));
    // shift middle part together
    f = f * f * (3. - 2. * f);
    // scale between red and violet
    // experimental values based on good composition behaviour
    f = max(0., f*0.9 - 0.08);
    // XXX One could save the instructions for the saturation part here
    return hsv2rgb(vec3(f, 1., v));
}
