// http://www.iquilezles.org/www/articles/voronoise/voronoise.htm

float vnoise1(in vec2 x, float u = 1., float v = 1.)
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    float k = 1.0 + 63.0*pow(1.0-v,4.0);
    float va = 0.0, wt = 0.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2  g = vec2( float(i), float(j) );
        vec3  o = hash3( p + g );
        vec2  r = g - f + o.xy;
        float w = pow( 1.0-smoothstep(0.0,1.414,sqrt(dot(r,r))), k );
        va += w*o.z;
        wt += w;
    }

    return va/wt;
}

vec2 vnoise2(in vec2 x, float u = 1., float v = 1.)
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    float k = 1.0 + 63.0*pow(1.0-v,4.0);
    vec2 va = vec2(0.0);
    float wt = 0.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2  g = vec2( float(i), float(j) );
        vec2  o = hash2( p + g );
        vec2  r = g - f + o;
        float w = pow( 1.0-smoothstep(0.0,1.414,sqrt(dot(r,r))), k );
        va += w*hash2( (p + g)*1.11 + 3. );
        wt += w;
    }

    return va/wt;
}

vec3 vnoise3(in vec2 x, float u = 1., float v = 1.)
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    float k = 1.0 + 63.0*pow(1.0-v,4.0);
    vec3 va = vec3(0.0);
    float wt = 0.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2  g = vec2( float(i), float(j) );
        vec2  o = hash2( p + g );
        vec2  r = g - f + o;
        float w = pow( 1.0-smoothstep(0.0,1.414,sqrt(dot(r,r))), k );
        va += w*hash3( (p + g)*1.11 + 3. );
        wt += w;
    }

    return va/wt;
}
