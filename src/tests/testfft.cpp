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

template <typename F>
int testConv2()
{
    MO_PRINT("\ntestConv2 " << typeid(F).name());

    std::vector<F>
            inp, kernel, conv;

    inp.resize(7);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = F(i+1);

    kernel.resize(2);
    for (size_t i=0; i<kernel.size(); ++i)
        kernel[i] = i == 1;

    conv.resize(inp.size() + kernel.size());

    std::cout << "input:   "; print(inp);
    std::cout << "kernel:  "; print(kernel);
    std::cout << "conv:    "; print(conv);

    MO::MATH::Convolution<F> c;
    c.setKernel(&kernel[0], kernel.size());

    c.convolve(&conv[0], &inp[0], inp.size());
    std::cout << "conv:    "; print(conv);

    c.convolveComplex(&conv[0], &inp[0], inp.size());
    std::cout << "conv:    "; print(conv);

    return 0;
}

template <typename F>
int testFft2()
{
    MO_PRINT("\ntestFft2 " << typeid(F).name());

    std::vector<F>
            inp,
            rinp, cinp,
            rinp2, cinp2;

    inp.resize(16);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = F(i+1);

    std::cout << "input:    "; print(inp);

    rinp.resize(MO::MATH::OouraFFT<F>::ComplexSize(inp.size()));
    cinp.resize(rinp.size());

    MO::MATH::OouraFFT<F> fft;
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

template <typename F>
int testConv3()
{
    MO_PRINT("\ntestConv3");

    std::vector<F>
            inp, kernel, conv,
            rinp, cinp, rkernel, ckernel, rconv, cconv;

    inp.resize(16);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = i < 8 ? F(i+1) : 0.f;

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

    MO::MATH::Convolution<F> c;
    c.setKernel(kernel.data(), 8);

    c.convolve(conv.data(), inp.data(), 8);
    std::cout << "conv:    "; print(conv);

    MO::MATH::OouraFFT<F> fft;
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


template <typename F>
int testConv4()
{
    MO_PRINT("\ntestConv4 " << typeid(F).name());

    std::vector<F>
            inp, kernel, conv;

    inp.resize(16);
    for (size_t i=0; i<inp.size(); ++i)
        inp[i] = i < 8 ? F(i+1) : F(0);

    kernel.resize(inp.size());
    for (size_t i=0; i<kernel.size(); ++i)
        kernel[i] = std::max(0, 4-int(i));

    conv.resize(inp.size());

    std::cout << "input:   "; print(inp);
    std::cout << "kernel:  "; print(kernel);

    MO::MATH::Convolution<F> c;
    c.setKernel(kernel.data(), 8);

    c.convolve(conv.data(), inp.data(), 8);
    std::cout << "conv:    "; print(conv);

    MO_PRINT("FFT");

    MO::MATH::OouraFFT<F> fft;
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
    //testConv2<float>();
    //testConv4<float>();
    //testConv4<double>();
    return 0;
}


} // namespace MO
