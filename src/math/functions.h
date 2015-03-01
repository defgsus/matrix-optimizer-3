/** @file math/functions.h

    @brief generic math functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/24/2014</p>
*/

#ifndef MOSCR_MATH_FUNCTIONS_H
#define MOSCR_MATH_FUNCTIONS_H

#include <cmath>

namespace MO {
namespace MATH {

/** Quantizes @p value to the given @p steps.
    @p Float can be an integer as well. */
template <typename Float>
Float quant(Float value, Float steps)
{
    return std::floor(value / steps) * steps;
}

/** Returns A modulo B for floating point numbers */
template <typename Float>
Float modulo(Float A, Float B)
{
    return std::fmod(A, B);
}

/** Returns sign-corrected A modulo B for floating point numbers */
template <typename Float>
Float moduloSigned(Float A, Float B)
{
    if (A >= 0)
        return std::fmod(A, B);
    else
        return B - std::fmod(-A, B);
}

/** Returns fractional part of float x */
template <typename F>
F frac(F x)
{
    return x - std::floor(x);
}


/** Fast tanh approximation,
    input range [-3,3], output range [-1,1] */
template <typename F>
F fast_tanh(F x)
{
    if (x < -3)
        return -1;
    else if (x > 3)
        return 1;
    else
        return x * ( F(27) + x * x ) / ( F(27) + F(9) * x * x );
}

/** Fast atan aproximation
    XXX not tested yet
    https://github.com/micknoise/Maximilian/blob/master/maximilian.h */
template <typename F>
F fast_atan(F x)
{
    return (x / (F(1) + F(0.28) * (x * x)));
}


/* below (max/min/clip) is from
 * Laurent de Soras: http://musicdsp.org/showArchiveComment.php?ArchiveID=81
 * "It may reduce accuracy for small numbers. I.e. if you clip to [-1; 1],
 * fractional part of the result will be quantized to 23 bits (or more,
 * depending on the bit depth of the temporary results).
 * Thus, 1e-20 will be rounded to 0. The other (positive) side effect is
 * the denormal number elimination. */

/** Branchless maximum */
template <typename F>
F bl_max(F x, F a)
{
    x -= a;
    x += std::abs(x);
    x *= 0.5;
    x += a;
    return x;
}

/** Branchless minimum */
template <typename F>
F bl_min(F x, F b)
{
    x = b - x;
    x += std::abs(x);
    x *= 0.5;
    x = b - x;
    return x;
}

/** Branchless clipping */
template <typename F>
F bl_clip(F x, F a, F b)
{
    const F x1 = std::abs(x-a),
            x2 = std::abs(x-b);
    x = x1 + (a + b);
    x -= x2;
    x *= 0.5;
    return x;
}



} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_FUNCTIONS_H
