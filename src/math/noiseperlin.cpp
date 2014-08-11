/** @file noiseperlin.cpp

    @brief coherent noise function over 1, 2 or 3 dimensions

    <p>org. copyright Ken Perlin</p>

    <p>(c) 2012, 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#include <cmath>

#include "noiseperlin.h"
//#include "random.h"
#include <random>

namespace MO {
namespace MATH {

NoisePerlin::NoisePerlin(unsigned int seed)
{
    p = 0;
    g1 = g2 = g3 = 0;

    init_();
    this->seed(seed);
}

NoisePerlin::~NoisePerlin()
{
    free(g1);
    free(g2);
    free(g3);
    free(p);
}


void NoisePerlin::init_()
{
    B = 0x100;
    BM = 0xff;

    N = 0x1000;
    NM = 0xfff;

    NP = 12;

    p = (int*) realloc(p, (B + B + 2) * sizeof(int));
    g1 = (Double*) realloc(g1, (B + B + 2) * sizeof(Double));
    g2 = (Double*) realloc(g2, ((B + B + 2) * 2) * sizeof(Double));
    g3 = (Double*) realloc(g3, ((B + B + 2) * 3) * sizeof(Double));

}



void NoisePerlin::seed(unsigned int seed)
{
    std::mt19937 rnd(seed);

    int i, j, k;

    for (i = 0 ; i < B ; i++)
    {
        p[i] = i;

        g1[i] = (Double)(((int)rnd() % (B + B)) - B) / B;

        for (j = 0 ; j < 2 ; j++)
            g2[i*2+j] = (Double)(((int)rnd() % (B + B)) - B) / B;
        normalize2_(&g2[i*2]);

        for (j = 0 ; j < 3 ; j++)
            g3[i*3+j] = (Double)(((int)rnd() % (B + B)) - B) / B;
        normalize3_(&g3[i*3]);
    }

    // shuffle indices
    while (--i)
    {
        k = p[i];
        p[i] = p[j = rnd() % B];
        p[j] = k;
    }

    for (i = 0 ; i < B + 2 ; i++)
    {
        p[B + i] = p[i];
        g1[B + i] = g1[i];
        for (j = 0 ; j < 2 ; j++)
            g2[(B + i)*2+j] = g2[i*2+j];
        for (j = 0 ; j < 3 ; j++)
            g3[(B + i)*3+j] = g3[i*3+j];
    }
}

void NoisePerlin::normalize2_(Double v[2]) const
{
    Double s;

    s = std::sqrt(v[0] * v[0] + v[1] * v[1]);
    v[0] = v[0] / s;
    v[1] = v[1] / s;
}

void NoisePerlin::normalize3_(Double v[3]) const
{
    Double s;

    s = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] = v[0] / s;
    v[1] = v[1] / s;
    v[2] = v[2] / s;
}



Double NoisePerlin::noise(Double arg_x) const
{
    int bx0, bx1;
    Double rx0, rx1, sx, t, u, v;

    prepare_(arg_x, t, bx0,bx1, rx0,rx1);

    sx = curve_(rx0);

    u = rx0 * g1[ p[ bx0 ] ];
    v = rx1 * g1[ p[ bx1 ] ];

    return lerp_(sx, u, v);
}



Double NoisePerlin::noise(Double arg_x, Double arg_y) const
{
    int bx0, bx1, by0, by1, b00, b10, b01, b11;
    Double rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
    register int i, j;

    prepare_(arg_x, t, bx0,bx1, rx0,rx1);
    prepare_(arg_y, t, by0,by1, ry0,ry1);

    i = p[ bx0 ];
    j = p[ bx1 ];

    b00 = p[ i + by0 ];
    b10 = p[ j + by0 ];
    b01 = p[ i + by1 ];
    b11 = p[ j + by1 ];

    sx = curve_(rx0);
    sy = curve_(ry0);

    q = &g2[ b00 ]; u = rx0 * q[0] + ry0 * q[1];
    q = &g2[ b10 ]; v = rx1 * q[0] + ry0 * q[1];
    a = lerp_(sx, u, v);

    q = &g2[ b01 ] ; u = rx0 * q[0] + ry1 * q[1];
    q = &g2[ b11 ] ; v = rx1 * q[0] + ry1 * q[1];
    b = lerp_(sx, u, v);

    return lerp_(sy, a, b);
}


Double NoisePerlin::noise(Double arg_x, Double arg_y, Double arg_z) const
{
    int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
    Double rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
    register int i, j;

    prepare_(arg_x, t, bx0,bx1, rx0,rx1);
    prepare_(arg_y, t, by0,by1, ry0,ry1);
    prepare_(arg_z, t, bz0,bz1, rz0,rz1);

    i = p[ bx0 ];
    j = p[ bx1 ];

    b00 = p[ i + by0 ];
    b10 = p[ j + by0 ];
    b01 = p[ i + by1 ];
    b11 = p[ j + by1 ];

    t  = curve_(rx0);
    sy = curve_(ry0);
    sz = curve_(rz0);

    q = &g3[ b00 + bz0 ] ; u = rx0 * q[0] + ry0 * q[1] + rz0 * q[2];
    q = &g3[ b10 + bz0 ] ; v = rx1 * q[0] + ry0 * q[1] + rz0 * q[2];
    a = lerp_(t, u, v);

    q = &g3[ b01 + bz0 ] ; u = rx0 * q[0] + ry1 * q[1] + rz0 * q[2];
    q = &g3[ b11 + bz0 ] ; v = rx1 * q[0] + ry1 * q[1] + rz0 * q[2];
    b = lerp_(t, u, v);

    c = lerp_(sy, a, b);

    q = &g3[ b00 + bz1 ] ; u = rx0 * q[0] + ry0 * q[1] + rz1 * q[2];
    q = &g3[ b10 + bz1 ] ; v = rx1 * q[0] + ry0 * q[1] + rz1 * q[2];
    a = lerp_(t, u, v);

    q = &g3[ b01 + bz1 ] ; u = rx0 * q[0] + ry1 * q[1] + rz1 * q[2];
    q = &g3[ b11 + bz1 ] ; v = rx1 * q[0] + ry1 * q[1] + rz1 * q[2];
    b = lerp_(t, u, v);

    d = lerp_(sy, a, b);

    return lerp_(sz, c, d);
}

Double NoisePerlin::noiseoct(Double arg_x, uint oct) const
{
    Double n = 0.0, amp = 1.0;
    for (uint i=0; i<oct; ++i)
    {
        n += noise(arg_x) * amp;
        arg_x *= 2.0;
        amp *= 0.5;
    }
    return n;
}


Double NoisePerlin::noiseoct(Double arg_x, Double arg_y, uint oct) const
{
    Double n = 0.0, amp = 1.0;
    for (uint i=0; i<oct; ++i)
    {
        n += noise(arg_x, arg_y) * amp;
        arg_x *= 2.0;
        arg_y *= 2.0;
        amp *= 0.5;
    }
    return n;
}

Double NoisePerlin::noiseoct(Double arg_x, Double arg_y, Double arg_z, uint oct) const
{
    Double n = 0.0, amp = 1.0;
    for (uint i=0; i<oct; ++i)
    {
        n += noise(arg_x, arg_y, arg_z) * amp;
        arg_x *= 2.0;
        arg_y *= 2.0;
        arg_z *= 2.0;
        amp *= 0.5;
    }
    return n;
}


Double NoisePerlin::noisef(Double arg_x, Double arg_y, Double scale, int oct) const
{
    Double n = 0.0;
    for (int i=1; i<oct; ++i)
    {
        n = noise(arg_x, arg_y);
        arg_x += noise(n, arg_y) * scale;
        arg_y += noise(n, arg_x) * scale;
    }
    n = noise(arg_x, arg_y);
    return n;
}

} // namespace MATH
} // namespace MO
