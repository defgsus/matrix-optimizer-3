/** @file convolution.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.05.2015</p>
*/

#ifndef MOSRC_MATH_CONVOLUTION_H
#define MOSRC_MATH_CONVOLUTION_H

#include <vector>
#include <algorithm>

#include "fft2.h"
#include "types/int.h" // for nextPowerOfTwo()

namespace MO {
namespace MATH {

template <typename F>
class Convolution
{
public:

    Convolution()
    { }

    bool isEmpty() const { return p_kernel_.empty(); }

    /** Returns the size in samples of the current kernel. */
    size_t kernelSize() const { return p_kernel_.size(); }

    /** Returns the samples of the current kernel.
        Invalid if isEmpty(). */
    const F* kernel() const { return &p_kernel_[0]; }

    F kernel(size_t i) const { return p_kernel_[i]; }

    // ---------- setter -----------

    void clear();

    /** Copies the data to kernel */
    void setKernel(const F * data, size_t num, size_t stride = 1);

    /** Resizes the kernel, used in conjunction with F* kernel() */
    void setKernelSize(size_t num) { p_kernel_.resize(num); }

    /** Direct write access */
    F* kernel() { return &p_kernel_[0]; }

    /** Zeroes all values in the kernal for which their
        absolute value is below @p minValue */
    void setKernelZeroBelow(F minValue);

    /** Convolve @p num samples in @p src with the current kernel into @p dst.
        @p dst must have space for @p num + kernelSize() samples.
        Invalid if isEmpty(). */
    void convolve(F * dst, const F * src, size_t num);

    /** Convolve @p num samples in @p src with the current kernel into @p dst.
        @p dst must have space for @p num + kernelSize() samples.
        Invalid if isEmpty().
        Much faster than convolve() but needs some more memory as well. */
    void convolveComplex(F * dst, const F * src, size_t num);

    // ----------------- helper ----------------

    static void clear(F * data, size_t num, F value = F(0));
    static void normalize(F * data, size_t num, F amp = F(1));

private:

    std::vector<F>
        p_kernel_,
        p_ikernel_, p_isrc_, p_iconv_;
    OouraFFT<F>
        p_fft_;
};




// ---------------------- IMPL ----------------------

template <typename F>
void Convolution<F>::clear()
{
    p_kernel_.clear();
}

template <typename F>
void Convolution<F>::setKernel(const F * data, size_t num, size_t stride)
{
    p_kernel_.resize(num);
    if (stride == 1)
    {
        memcpy(&p_kernel_[0], data, num * sizeof(F));
    }
    else
    {
        for (size_t i=0; i<num; ++i, data += stride)
            p_kernel_[i] = *data;
    }
}

template <typename F>

void Convolution<F>::setKernelZeroBelow(F mi)
{
    F * k = &p_kernel_[0];
    for (size_t i=0; i<kernelSize(); ++i, ++k)
        if (std::abs(*k) < mi)
            *k = F(0);
}

template <typename F>
void Convolution<F>::clear(F * data, size_t num, F value)
{
    for (size_t i=0; i<num; ++i)
        data[i] = value;
}

template <typename F>
void Convolution<F>::normalize(F * data, size_t num, F amp)
{
    F ma = F(0);
    for (size_t i=0; i<num; ++i)
        ma = std::max(ma, data[i]);

    if (ma)
    {
        ma = amp / ma;
        for (size_t i=0; i<num; ++i)
            data[i] *= ma;
    }
}


template <typename F>
void Convolution<F>::convolve(F * dst, const F * src, size_t num)
{
    clear(dst, num + kernelSize());

    for (size_t i=0; i<kernelSize(); ++i)
        if (kernel(i))
            for (size_t j=0; j<num; ++j)
                dst[j + i] += kernel(i) * src[j];
}

template <typename F>
void Convolution<F>::convolveComplex(F * dst, const F * src, size_t num)
{
    // size of fft
    const size_t cnum = nextPowerOfTwo( num + kernelSize() );

    // buffer for fft and multiply
    p_isrc_.resize(cnum),
    p_ikernel_.resize(cnum);
    p_iconv_.resize(cnum);

    if (p_fft_.size() != cnum)
        p_fft_.init(cnum);

    // copy src and zero-pad
    size_t i;
    for (i = 0; i<num; ++i)
        p_isrc_[i] = src[i];
    for (; i < cnum; ++i)
        p_isrc_[i] = F(0);

    // copy kernel and zero-pad
    for (i=0; i<kernelSize(); ++i)
        p_ikernel_[i] = kernel(i);
    for (; i < cnum; ++i)
        p_ikernel_[i] = F(0);

    p_fft_.fft(p_isrc_.data());
    p_fft_.fft(p_ikernel_.data());

    // multiply with kernel (complex multiplication)
    p_fft_.complexMultiply(p_iconv_.data(), p_isrc_.data(), p_ikernel_.data());

    p_fft_.ifft(p_iconv_.data());

    // copy to dst
    for (size_t i=0; i<num + kernelSize(); ++i)
        *dst++ = p_iconv_[i];
}



} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_CONVOLUTION_H
