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
#include "types/int.h"

namespace MO {
namespace MATH {


/** Inplace FFT of real values.
    Input is @p num real values (r0,r1,r2,r3..).
    Output is num/2 real and num/2 imag values (r0,r1,r2,...,i2,i1,i0)
    */
void real_fft(Float * data, uint num);
void real_fft(Double * data, uint num);

/** Inplace inverse FFT of real and imaginary values.
    Input is @p num/2 real values (r0,r1,r2...) and num/2 imaginary values (...i2,i1,i0)
    Output is @p num real values */
void ifft(Float * data, uint num);
void ifft(Double * data, uint num);

/** Transforms a buffer from fft output into amplitude and phase information.
    @p data is supposed to point at @p num floats.
    Input is (r0,r1,r2,...,i2,i1,i0).
    Output is (a0,a1,a2,...,p0,p1,p2).
    The phases are in radians. */
void get_amplitude_phase(Float * data, uint num);
void get_amplitude_phase(Double * data, uint num);



/** Wrapper for standalone functions with associated buffer */
template <typename F>
class Fft
{
public:
    Fft(uint size = 1024)
        : buffer_   (size)
    { }

    // ------------ getter ----------------

    uint size() const { return buffer_.size(); }
    uint half() const { return buffer_.size() >> 1; }

    /** Read access to the internal buffer.
        Contains size() real values and size() imaginary values. */
    const F * buffer() const { return &buffer_[0]; }

    // ------------ setter ----------------

    /** Sets the size of the buffer (rounded to the next power of two).
        Also zeroes all data. */
    void setSize(uint size) { buffer_.resize(nextPowerOfTwo(size)); clear(); }

    /** Zeroes the buffer */
    void clear() { for (auto & v : buffer_) v = F(0); }

    /** Write access to the internal buffer */
    F * buffer() { return &buffer_[0]; }

    // ------------ fft -------------------

    /** Perform real to fourier transform of the real data in buffer().
        Input is size() real values (r0,r1,r2,r3..).
        The output is in buffer(). */
    void fft() { real_fft(buffer(), size()); }

    /** Perform real to fourier transform of given data.
        Input is size() real values (r0,r1,r2,r3..).
        The output is in buffer(). */
    void fft(const F * real_data)
    {
        memcpy(&buffer_[0], real_data, size() * sizeof(F));
        real_fft(buffer(), size());
    }

    /** Perform inverse fourier transform of current buffer.
        Result is size() real values in buffer() */
    void ifft() { MATH::ifft(&buffer_[0], size()); }

    /** Transforms the ft in buffer() into amplitude and phase */
    void getAmplitudeAndPhase() { get_amplitude_phase(&buffer_[0], size()); }

private:

    std::vector<F> buffer_;
};




} // namespace MATH
} // namespace MO


#endif // MOSRC_MATH_FFT_H
