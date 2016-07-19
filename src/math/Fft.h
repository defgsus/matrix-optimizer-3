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

/** Multiplies complex numbers in @p A and @p B.
    Input for both @p A and @p B is num/2 real and num/2 imag values
    (r0,r1,r2,...,i2,i1,i0)
    Multiplication is stored in @p dst, which can be equal to @p A and/or @p B */
template <typename F>
void complex_multiply(F * dst, const F * A, const F * B, uint num);

/** Divides complex numbers @p A by @p B.
    Input for both @p A and @p B is num/2 real and num/2 imag values
    (r0,r1,r2,...,i2,i1,i0)
    Quotients are stored in @p dst, which can be equal to @p A and/or @p B */
template <typename F>
void complex_divide(F * dst, const F * A, const F * B, uint num);


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
    uint halfSize() const { return buffer_.size() >> 1; }

    /** Read access to the internal buffer.
        Contains halfSize() real values and halfSize() imaginary values. */
    const F * buffer() const { return &buffer_[0]; }

    /** Read idx'th sample in buffer */
    F buffer(size_t idx) const { return buffer_[idx]; }

    // ------------ setter ----------------

    /** Sets the size of the buffer (rounded to the next power of two).
        Also zeroes all data. */
    void setSize(uint size) { buffer_.resize(nextPowerOfTwo(size)); clear(); }

    /** Zeroes the buffer */
    void clear() { for (auto & v : buffer_) v = F(0); }

    /** Write access to the internal buffer of size() values */
    F * buffer() { return &buffer_[0]; }

    // ------------ fft -------------------

    /** Perform real to fourier transform of the real data in buffer().
        Input is size() real values (r0,r1,r2,r3..).
        The output is in buffer(). */
    void fft() { real_fft(buffer(), size()); }

    /** Perform real to fourier transform of given data.
        Input is size() real values (r0,r1,r2,r3..).
        The output is in buffer() containing
        each halfSize() real and imaginary values. */
    void fft(const F * real_data)
    {
        memcpy(&buffer_[0], real_data, size() * sizeof(F));
        real_fft(buffer(), size());
    }

    /** Perform inverse fourier transform of current buffer.
        Result is size() real values in buffer() */
    void ifft() { MATH::ifft(&buffer_[0], size()); }

    /** Transforms the ft in buffer() into amplitude and phase */
    void toAmplitudeAndPhase() { get_amplitude_phase(&buffer_[0], size()); }

private:

    std::vector<F> buffer_;
};



// -------------- IMPL ----------------

template <typename F>
void complex_multiply(F * dst, const F* A, const F* B, uint num)
{
    size_t num2 = num / 2;
    for (size_t i=0; i<num2; ++i)
    {
        size_t j = num - 1 - i;
        F a_r = A[i],
          a_i = A[j],
          b_r = B[i],
          b_i = B[j],

          re = a_r * b_r - a_i * b_i,
          im = a_r * b_i + a_i * b_r;

        dst[i] = re;
        dst[j] = im;
    }
}

template <typename F>
void complex_divide(F * dst, const F* A, const F* B, uint num)
{
    size_t num2 = num / 2;
    for (size_t i=0; i<num2; ++i)
    {
        size_t j = num - 1 - i;
        F b_r = B[i],
          b_i = B[j],
          a_r = A[i],
          a_i = A[j],

          n = b_r * b_r + b_i * b_i,

          re = (a_r * b_r + a_i * b_i) / n,
          im = (a_i * b_r + a_r * b_i) / n;

        dst[i] = re;
        dst[j] = im;
    }
}


} // namespace MATH
} // namespace MO


#endif // MOSRC_MATH_FFT_H
