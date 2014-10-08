/** @file fft.h

    @brief fft and ifft functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/8/2014</p>
*/

#ifndef MOSRC_MATH_FFT_H
#define MOSRC_MATH_FFT_H

#include <types/float.h>

namespace MO {
namespace MATH {


/** Inplace FFT of real values.
    Input is @p num real values (r0,r1,r2,r3..).
    Output is @p num * 2 real and imag values (r0,r1,r2,...,i0,i1,i2)
    normalized by array length.
    */
void realfft(Float * data, uint num);
void realfft(Double * data, uint num);


} // namespace MATH
} // namespace MO


#endif // MOSRC_MATH_FFT_H
