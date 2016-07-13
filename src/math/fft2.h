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
class OouraFFT
{
public:
  OouraFFT();

  /**
   * @brief Initializes the FFT object
   * @param size Size of the real input (must be power 2)
   */
  void init(size_t size);

  /**
   * @brief Performs the forward FFT
   * @param data The real input data (has to be of the length as specified in init())
   * @param re The real part of the complex output (has to be of length as returned by ComplexSize())
   * @param im The imaginary part of the complex output (has to be of length as returned by ComplexSize())
   */
  void fft(const float* data, float* re, float* im);

  /**
   * @brief Performs the inverse FFT
   * @param data The real output data (has to be of the length as specified in init())
   * @param re The real part of the complex input (has to be of length as returned by ComplexSize())
   * @param im The imaginary part of the complex input (has to be of length as returned by ComplexSize())
   */
  void ifft(float* data, const float* re, const float* im);

  /**
   * @brief Calculates the necessary size of the real/imaginary complex arrays
   * @param size The size of the real data
   * @return The size of the real/imaginary complex arrays
   */
  static size_t ComplexSize(size_t size)
  {
    return (size / 2) + 1;
  }

private:
  size_t _size;
  std::vector<int> _ip;
  std::vector<double> _w;
  std::vector<double> _buffer;

  // The original FFT routines by Takuya Ooura (see http://momonga.t.u-tokyo.ac.jp/~ooura/fft.html)
  void rdft(int n, int isgn, double *a, int *ip, double *w);
  void makewt(int nw, int *ip, double *w);
  void makect(int nc, int *ip, double *c);
  void bitrv2(int n, int *ip, double *a);
  void cftfsub(int n, double *a, double *w);
  void cftbsub(int n, double *a, double *w);
  void rftfsub(int n, double *a, int nc, double *c);
  void rftbsub(int n, double *a, int nc, double *c);
  void cft1st(int n, double *a, double *w);
  void cftmdl(int n, int l, double *a, double *w);

  // Prevent uncontrolled usage
  OouraFFT(const OouraFFT&);
  OouraFFT& operator=(const OouraFFT&);
};


} // namespace MATH
} // namespace MO


#endif // MOSRC_MATH_FFT2_H
