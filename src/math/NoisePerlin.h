/** @file noiseperlin.h

    @brief coherent noise function over 1, 2 or 3 dimensions

    <p>org. copyright Ken Perlin</p>

    <p>(c) 2012, 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#ifndef MOSRC_MATH_NOISEPERLIN_H
#define MOSRC_MATH_NOISEPERLIN_H

#include <QHash>

#include "types/float.h"
#include "types/vector.h"

namespace MO {
namespace MATH {

//#define MO_NP_VORONOI_BUFFER

/** The age-old perlin noise generator as class.

    <p>C++ed from original code by Ken Perlin</p>
    */
class NoisePerlin
{
public:

    typedef int64_t Int;

    /** constructor with optional seed. <br>
        seed can be later (re-)set with seed() */
    NoisePerlin(Int seed = 1007);
    ~NoisePerlin();

    /** sets the initial seed value */
    void seed(Int seed);

    /** Returns the noise value for a given number */
    Double noise(Double arg_x) const;

    /** Returns the noise value for a 2d-vector */
    Double noise(Double arg_x, Double arg_y) const;

    /** Returns the noise value for a 3d-vector */
    Double noise(Double arg_x, Double arg_y, Double arg_z) const;

    /** Returns octaved noise (ocatve * 2, amplitude * 0.5) */
    Double noiseoct(Double arg_x, uint oct) const;

    /** Returns octaved noise (ocatve * 2, amplitude * 0.5) */
    Double noiseoct(Double arg_x, Double arg_y, uint oct) const;

    /** Returns octaved noise (ocatve * 2, amplitude * 0.5) */
    Double noiseoct(Double arg_x, Double arg_y, Double arg_z, uint oct) const;

    /** Returns fractal noise.
        @todo make noisef() more general? */
    Double noisef(Double arg_x, Double arg_y, Double scale, int oct) const;


    Double random(int x) const;
    Double random(int x, int y) const;
    Double random(int x, int y, int z) const;

    Vec2 random2(int x, int y) const;
    Vec3 random3(int x, int y, int z) const;

    /** Returns the distance to the closest point in a jittered grid */
    Double voronoi(Double x, Double y);
    Double voronoi(Double x, Double y, Double z);

    /** Returns the distance to the closest point in a jittered grid,
        while smoothly fading between closest points. */
    Double s_voronoi(Double x, Double y, Double sm);
    Double s_voronoi(Double x, Double y, Double z, Double sm);

    Vec3 voronoiCellPos(int x, int y, int z);

private:

    void init_();

    Double curve_(Double t) const { return t * t * (3.0 - 2.0 * t); }
    Double lerp_(Double t, Double a, Double b) const { return a + t * (b - a); }

    void normalize2_(Double v[2]) const;
    void normalize3_(Double v[3]) const;

    void prepare_(Double arg, Double& t, Int& b0, Int& b1, Double& r0, Double& r1) const
    {
        t = arg + N;
        b0 = ((Int)t) & BM;
        b1 = (b0+1) & BM;
        r0 = t - (Int)t;
        r1 = r0 - 1.0;
    }

    void prepare_(Double arg, Double& t, Int& b0, Int& b1) const
    {
        t = arg + N;
        b0 = ((Int)t) & BM;
        b1 = (b0+1) & BM;
    }


    Int *p,
        B, BM,
        N, NP, NM;
    Double *g1, *g2, *g3;

#ifdef MO_NP_VORONOI_BUFFER
    QHash<int, Vec3> voro_;
#endif
};

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_NOISEPERLIN_H
