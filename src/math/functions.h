/** @file functions.h

    @brief generic math functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/24/2014</p>
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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

#endif // FUNCTIONS_H
