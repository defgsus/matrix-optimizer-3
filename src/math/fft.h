/** @file fft.h

    @brief fft and ifft functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/8/2014</p>
*/

#ifndef MOSRC_MATH_FFT_H
#define MOSRC_MATH_FFT_H

#include <vector>

#include "types/float.h"

namespace MO {
namespace MATH {


/** Inplace FFT of real values.
    Input is @p num real values (r0,r1,r2,r3..).
    Output is @p num * 2 real and imag values (r0,r1,r2,...,i0,i1,i2)
    */
void real_fft(Float * data, uint num);
void real_fft(Double * data, uint num);

/** Transforms a buffer from fft output into amplitude and phase information.
    @p data is supposed to point at @p num * 2 floats.
    Input is (r0,r1,r2,...,i0,i1,i2).
    Output is (a0,a1,a2,...,p0,p1,p2).
    The phases are in radians. */
void get_amplitude_phase(Float * data, uint num);
void get_amplitude_phase(Double * data, uint num);



/** Wrapper for standalone functions with associated buffer */
template <typename F>
class Fft
{
public:
    Fft(uint size)
        : buffer_   (size*2)
    { }

    // ------------ getter ----------------

    uint size() const { return buffer_.size() / 2; }

    /** Read access to the internal buffer.
        Contains size() real values and size() imaginary values. */
    const F * buffer() const { return &buffer_[0]; }

    // ------------ setter ----------------

    void setSize(uint size) { buffer_.resize(size * 2); }

    /** Write access to the internal buffer */
    F * buffer() { return &buffer_[0]; }

    // ------------ fft -------------------

    /** Perform real to real/imag fourier transform.
        Input is size() real values (r0,r1,r2,r3..).
        The output is in buffer(). */
    void fft(const F * real_data)
    {
        memcpy(&buffer_[0], real_data, size() * sizeof(F));
        realfft(buffer(), size());
    }

    /** Transforms the ft in buffer() into amplitude and phase */
    void getAmplitudeAndPhase() { get_amplitude_phase(&buffer_[0], size()); }

private:

    std::vector<F> buffer_;
};




} // namespace MATH
} // namespace MO


#endif // MOSRC_MATH_FFT_H
