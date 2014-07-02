/** @file functions.h

    @brief generic math functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/24/2014</p>
*/

#ifndef MOSCR_FUNCTIONS_H
#define MOSCR_FUNCTIONS_H

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


} // namespace MATH
} // namespace MO

#endif // MOSRC_FUNCTIONS_H
