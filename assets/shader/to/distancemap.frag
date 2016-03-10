#include <to/header>

// define DISTANCE: 1-1000
// define DISTANCE_DIAG: diagonal of DISTANCE ( DISTANCE * sqrt(2) )

void main()
{
    float minD = DISTANCE_DIAG;
    for (int j=-DISTANCE; j<=DISTANCE; ++j)
    for (int i=-DISTANCE; i<=DISTANCE; ++i)
    {
        vec2 ij = vec2(float(i), float(j));
        float p = texture(u_tex, v_texCoord + u_resolution.zw * ij).x;
        // if pixel is ON, determine minimum distance
        if (p >= .5)
        {
            float d = length(ij);
            minD = min(minD, d);
        }
    }

    float res = min(1., minD / float(DISTANCE_DIAG));
    fragColor = vec4(res, res, res, 1.);
}
