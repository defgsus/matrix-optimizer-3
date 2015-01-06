/** @file denormals.cpp

    @brief Denormals handling

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#ifdef __SSE__
#   include <xmmintrin.h>
#endif

#include "denormals.h"

namespace MO {
namespace MATH {

// https://en.wikipedia.org/wiki/Denormal_number

#ifndef _MM_DENORMALS_ZERO_MASK
#   define _MM_DENORMALS_ZERO_MASK   0x0040
#   define _MM_DENORMALS_ZERO_ON     0x0040
#   define _MM_DENORMALS_ZERO_OFF    0x0000

#   define _MM_SET_DENORMALS_ZERO_MODE(mode)                                   \
            _mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (mode))

#   define _MM_GET_DENORMALS_ZERO_MODE()                                       \
            (_mm_getcsr() & _MM_DENORMALS_ZERO_MASK)
#endif

void setDenormals(bool enable)
{
#ifdef __SSE__
    if (enable)
        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
    else
        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#else
    enable = enable;
#endif
}

} // namespace MATH
} // namespace MO
