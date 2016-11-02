// ==================================================================================
// Copyright (c) 2013 HiFi-LoFi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ==================================================================================

/* Templated and a few additions made for
   inplace-processing of data buffers - sb/2016 */

#ifndef MOSRC_MATH_FFT2_H
#define MOSRC_MATH_FFT2_H

#include <vector>
#include <cstddef>

namespace MO {
namespace MATH {

/**
* @class OouraFFT
* @brief FFT implementation based on the great radix-4 routines by Takuya Ooura
* This implementation is by
    https://github.com/HiFi-LoFi/KlangFalter/blob/master/Source/FFTConvolver/AudioFFT.h
*/
template <typename F>
class OouraFFT
{
public:

  OouraFFT();

  /**
   * @brief Initializes the FFT object
   * @param size Size of the real input (must be power 2)
   */
  void init(size_t size);
  void setSize(size_t size) { init(size); }

  /** The initialized size of the FFT object */
  size_t size() const { return size_; }

  /**
   * @brief Performs the forward FFT
   * @param data The real input data (has to be of the length as specified in init())
   * @param re The real part of the complex output
   *        (has to be of length as returned by ComplexSize())
   * @param im The imaginary part of the complex output
   *        (has to be of length as returned by ComplexSize())
   */
  void fft(const float* data, float* re, float* im);

  /**
   * @brief Performs the inverse FFT
   * @param data The real output data (has to be of the length as specified in init())
   * @param re The real part of the complex input
   *        (has to be of length as returned by ComplexSize())
   * @param im The imaginary part of the complex input
   *        (has to be of length as returned by ComplexSize())
   */
  void ifft(float* data, const float* re, const float* im);

  /**
   * @brief Performs the forward FFT on real numbers
   * @param data a pointer to size() real numbers
   * The calculation is done in-place, the result in @p data is
   * an interleaved real/imag format.
   */
  void fft(F* data);

  /**
   * @brief Performs the inverse FFT
   * @param data a pointer to size() real/image pairs
   * The calculation is done in-place, the result in @p data is
   * size() real numbers.
   */
  void ifft(F* data);

  /**
   * @brief Performs complex multiplication of a and b into dst
   * @param dst The destination in interleaved real/imag format
   * @param a The first multiplicant in interleaved real/imag format
   * @param b The second multiplicant in interleaved real/imag format
   * @note @p dst MUST NOT overlap with @p a or @p b
   */
  void complexMultiply(F* dst, const F* a, const F* b) const;

  /**
   * @brief Returns the real part from interleaved real/imag data
   * @param num The index from 0 to size() / 2 + 1
   */
  F getReal(const F* data, size_t num) const;
  /**
   * @brief Returns the imaginary part from interleaved real/imag data
   * @param num The index from 0 to size() / 2 + 1
   */
  F getImag(const F* data, size_t num) const;

  /**
   * @brief Sets the real part in interleaved real/imag data
   * @param num The index from 0 to size() / 2 + 1
   */
  void setReal(F* data, size_t num, F r) const;
  /**
   * @brief Sets the imaginary part in interleaved real/imag data
   * @param num The index from 0 to size() / 2 + 1
   */
  void setImag(F* data, size_t num, F i) const;

  /**
   * @brief Calculates the necessary size of the real/imaginary complex arrays
   * @param size The size of the real data
   * @return The size of the real/imaginary complex arrays
   */
  static size_t ComplexSize(size_t size)
  {
    return (size / 2) + 1;
  }

  size_t ComplexSize() const { return ComplexSize(size_); }

  /** @brief Multiply the complex data with the factor table.
      @param complexData The fourier transform in interleaved real/imag format
      @param factors A table of factors of length ComplexSize()
      */
  void multiply(F* complexData, const F* factors) const;

private:

  size_t size_;
  std::vector<int> ip_;
  std::vector<F> w_, buffer_;

  // The original FFT routines by Takuya Ooura
  // (see http://momonga.t.u-tokyo.ac.jp/~ooura/fft.html)
  void rdft(int n, int isgn, F *a, int *ip, F *w);
  void makewt(int nw, int *ip, F *w);
  void makect(int nc, int *ip, F *c);
  void bitrv2(int n, int *ip, F *a);
  void cftfsub(int n, F *a, F *w);
  void cftbsub(int n, F *a, F *w);
  void rftfsub(int n, F *a, int nc, F *c);
  void rftbsub(int n, F *a, int nc, F *c);
  void cft1st(int n, F *a, F *w);
  void cftmdl(int n, int l, F *a, F *w);
};


} // namespace MATH
} // namespace MO


#endif // MOSRC_MATH_FFT2_H
