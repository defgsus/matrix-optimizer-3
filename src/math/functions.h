/** @file functions.h

    @brief generic math functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/24/2014</p>
*/

#ifndef MOSCR_FUNCTIONS_H
#define MOSCR_FUNCTIONS_H

#include <cmath>

namespace MO {
namespace MATH {

template <typename Float>
Float quant(Float value, Float steps)
{
    return std::floor(value / steps) * steps;
}

} // namespace MATH
} // namespace MO

#endif // MOSRC_FUNCTIONS_H
