/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/13/2016</p>
*/

#include "testfft.h"

#include "math/fft.h"
#include "types/int.h"
#include "math/convolution.h"
#include "math/fft2.h"
#include "io/log.h"

namespace MO {


void ComplexMultiplyAccumulate(float* re,
                               float* im,
                               const float* reA,
                               const float* imA,
                               const float* reB,
                               const float* imB,
                               size_t len)
{
#if defined(FFTCONVOLVER_USE_SSE)
  const size_t end4 = 4 * (len / 4);
  for (size_t i=0; i<end4; i+=4)
  {
    const __m128 ra = _mm_load_ps(&reA[i]);
    const __m128 rb = _mm_load_ps(&reB[i]);
    const __m128 ia = _mm_load_ps(&imA[i]);
    const __m128 ib = _mm_load_ps(&imB[i]);
    __m128 real = _mm_load_ps(&re[i]);
    __m128 imag = _mm_load_ps(&im[i]);
    real = _mm_add_ps(real, _mm_mul_ps(ra, rb));
    real = _mm_sub_ps(real, _mm_mul_ps(ia, ib));
    _mm_store_ps(&re[i], real);
    imag = _mm_add_ps(imag, _mm_mul_ps(ra, ib));
    imag = _mm_add_ps(imag, _mm_mul_ps(ia, rb));
    _mm_store_ps(&im[i], imag);
  }
  for (size_t i=end4; i<len; ++i)
  {
    re[i] += reA[i] * reB[i] - imA[i] * imB[i];
    im[i] += reA[i] * imB[i] + imA[i] * reB[i];
  }
#else
  const size_t end4 = 4 * (len / 4);
  for (size_t i=0; i<end4; i+=4)
  {
    re[i+0] += reA[i+0] * reB[i+0] - imA[i+0] * imB[i+0];
    re[i+1] += reA[i+1] * reB[i+1] - imA[i+1] * imB[i+1];
    re[i+2] += reA[i+2] * reB[i+2] - imA[i+2] * imB[i+2];
    re[i+3] += reA[i+3] * reB[i+3] - imA[i+3] * imB[i+3];
    im[i+0] += reA[i+0] * imB[i+0] + imA[i+0] * reB[i+0];
    im[i+1] += reA[i+1] * imB[i+1] + imA[i+1] * reB[i+1];
    im[i+2] += reA[i+2] * imB[i+2] + imA[i+2] * reB[i+2];
    im[i+3] += reA[i+3] * imB[i+3] + imA[i+3] * reB[i+3];
  }
  for (size_t i=end4; i<len; ++i)
  {
    re[i] += reA[i] * reB[i] - imA[i] * imB[i];
    im[i] += reA[i] * imB[i] + imA[i] * reB[i];
  }
#endif
}


template <typename T>
void print(const std::vector<T>& v)
{
    std::cout << v.size() << ": ";
    for (size_t i=0; i<v.size(); ++i)
    {
        if (i != 0)
            std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << std::endl;
}

int testConv()
{
    std::vector<float>
            inp, kernel, conv,
            cinp, ckernel;

    inp.resize(7);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = i+1;

    kernel.resize(5);
    for (size_t i=0; i<kernel.size(); ++i)
        kernel[i] = i == 0;

    conv.resize(MO::nextPowerOfTwo(inp.size() + kernel.size()), 0.f);

    std::cout << "input:   "; print(inp);
    std::cout << "kernel:  "; print(kernel);
    std::cout << "conv:    "; print(conv);

    cinp.resize(conv.size(), 0.f);
    for (size_t i=0; i<inp.size(); ++i)
        cinp[i] = inp[i];

    ckernel.resize(conv.size(), 0.f);
    for (size_t i=0; i<ckernel.size(); ++i)
        ckernel[i] = kernel[i];

    MO::MATH::real_fft(&cinp[0], cinp.size());
    MO::MATH::real_fft(&ckernel[0], ckernel.size());
    MO::MATH::complex_multiply(&conv[0], &cinp[0], &ckernel[0], conv.size());
    MO::MATH::ifft(&conv[0], conv.size());

    std::cout << "cinput:  "; print(cinp);
    std::cout << "ckernel: "; print(ckernel);
    std::cout << "conv:    "; print(conv);

    return 0;
}

int testConv2()
{
    std::vector<float>
            inp, kernel, conv;

    inp.resize(7);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = float(i+1);

    kernel.resize(2);
    for (size_t i=0; i<kernel.size(); ++i)
        kernel[i] = i == 1;

    conv.resize(inp.size() + kernel.size());

    std::cout << "input:   "; print(inp);
    std::cout << "kernel:  "; print(kernel);
    std::cout << "conv:    "; print(conv);

    MO::MATH::Convolution<float> c;
    c.setKernel(&kernel[0], kernel.size());

    c.convolve(&conv[0], &inp[0], inp.size());
    std::cout << "conv:    "; print(conv);

    c.convolveComplex(&conv[0], &inp[0], inp.size());
    std::cout << "conv:    "; print(conv);

    return 0;
}

int testFft2()
{
    MO_PRINT("\ntestFft2");

    std::vector<float>
            inp,
            rinp, cinp,
            rinp2, cinp2;

    inp.resize(16);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = float(i+1);

    std::cout << "input:    "; print(inp);

    rinp.resize(MO::MATH::OouraFFT::ComplexSize(inp.size()));
    cinp.resize(rinp.size());

    MO::MATH::OouraFFT fft;
    fft.init(inp.size());

    MO_PRINT("split:");

    fft.fft(inp.data(), rinp.data(), cinp.data());

    std::cout << "r input:  "; print(rinp);
    std::cout << "i input:  "; print(cinp);

    MO_PRINT("unsplit:");

    fft.fft(inp.data());

    for (size_t i=0; i<rinp.size(); ++i)
    {
        rinp2.push_back(fft.getReal(inp.data(), i));
        cinp2.push_back(fft.getImag(inp.data(), i));
    }

    std::cout << "ft:       "; print(inp);
    std::cout << "r ft:     "; print(rinp2);
    std::cout << "i ft:     "; print(cinp2);

    return 0;
}

int testConv3()
{
    MO_PRINT("\ntestConv3");

    std::vector<float>
            inp, kernel, conv,
            rinp, cinp, rkernel, ckernel, rconv, cconv;

    inp.resize(16);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = i < 8 ? float(i+1) : 0.f;

    kernel.resize(inp.size());
    for (size_t i=0; i<kernel.size(); ++i)
        kernel[i] = i == 1;

    conv.resize(inp.size());

    rinp.resize(inp.size());
    cinp.resize(inp.size());
    rkernel.resize(inp.size());
    ckernel.resize(inp.size());
    rconv.resize(inp.size());
    cconv.resize(inp.size());


    std::cout << "input:   "; print(inp);
    std::cout << "kernel:  "; print(kernel);
    std::cout << "conv:    "; print(conv);

    MO::MATH::Convolution<float> c;
    c.setKernel(kernel.data(), 8);

    c.convolve(conv.data(), inp.data(), 8);
    std::cout << "conv:    "; print(conv);

    MO::MATH::OouraFFT fft;
    fft.init(inp.size());

    fft.fft(inp.data(), rinp.data(), cinp.data());

    std::cout << "r input:  "; print(rinp);
    std::cout << "i input:  "; print(cinp);

    fft.fft(kernel.data(), rkernel.data(), ckernel.data());

    std::cout << "r kernel: "; print(rkernel);
    std::cout << "i kernel: "; print(ckernel);

    MO_PRINT("convolve");

    ComplexMultiplyAccumulate(
                rconv.data(), cconv.data(),
                rinp.data(), cinp.data(),
                rkernel.data(), ckernel.data(), inp.size());

    std::cout << "r input:  "; print(rinp);
    std::cout << "i input:  "; print(cinp);

    fft.ifft(conv.data(), rconv.data(), cconv.data());

    std::cout << "conv:     "; print(conv);


    return 0;
}


int testConv4()
{
    MO_PRINT("\ntestConv4");

    std::vector<float>
            inp, kernel, conv;

    inp.resize(16);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = i < 8 ? float(i+1) : 0.f;

    kernel.resize(inp.size());
    for (size_t i=0; i<kernel.size(); ++i)
        kernel[i] = std::max(0, 4-int(i));

    conv.resize(inp.size());

    std::cout << "input:   "; print(inp);
    std::cout << "kernel:  "; print(kernel);

    MO::MATH::Convolution<float> c;
    c.setKernel(kernel.data(), 8);

    c.convolve(conv.data(), inp.data(), 8);
    std::cout << "conv:    "; print(conv);

    MO_PRINT("FFT");

    MO::MATH::OouraFFT fft;
    fft.init(inp.size());

    fft.fft(inp.data());
    fft.fft(kernel.data());

    std::cout << "input:   "; print(inp);
    std::cout << "kernel:  "; print(kernel);

    MO_PRINT("convolve");

    fft.complexMultiply(conv.data(), inp.data(), kernel.data());
    fft.ifft(conv.data());

    std::cout << "conv:    "; print(conv);

    return 0;
}

int TestFft::run()
{
    //testFft2();
    //testConv3();
    testConv4();
    return 0;
}


} // namespace MO
