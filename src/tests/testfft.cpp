/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/13/2016</p>
*/

#include <cassert>

#include <QDialog>
#include <QLayout>

#include "testfft.h"
#include "math/fft.h"
#include "types/int.h"
#include "math/convolution.h"
#include "math/fft2.h"
#include "gui/widget/soundfilewidget.h"
#include "audio/tool/soundfile.h"
#include "audio/tool/soundfilemanager.h"
#include "audio/tool/waveform.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/convolvebuffer.h"
#include "audio/audioplayer.h"
#include "io/application.h"
#include "io/time.h"
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

namespace {

// convolve as a whole
template <typename F>
void convolve1(std::vector<F>& conv,
               const std::vector<F>& input,
               const std::vector<F>& kernel)
{
    MATH::Convolution<F> c;
    c.setKernel(kernel.data(), kernel.size());

    conv.resize(input.size() + kernel.size());

    c.convolveComplex(conv.data(), input.data(), input.size());
}

// convolve by overlap-add
template <typename F>
void convolve2(std::vector<F>& conv,
               const std::vector<F>& input,
               const std::vector<F>& kernel)
{
    const size_t tileSize = kernel.size();
    const size_t numTiles = (input.size()+kernel.size()) / tileSize;
    const size_t convSize = nextPowerOfTwo(tileSize + kernel.size());

    conv.clear(); conv.resize(input.size() + kernel.size(), F(0));

    MATH::OouraFFT<F> fft;
    fft.init(convSize);

    std::vector<F>
            scratch(convSize, F(0)),
            cinput(convSize, F(0)),
            ckernel(convSize, F(0));

    // prepare ft kernel
    for (size_t i=0; i<kernel.size(); ++i)
        ckernel[i] = kernel[i];
    fft.fft(ckernel.data());

    size_t inPos = 0;
    for (size_t tile = 0; tile < numTiles; ++tile)
    {
        // chunk of input
        size_t i;
        for (i=0; i<tileSize; ++i)
            cinput[i] = inPos < input.size() ? input[inPos++] : F(0);
        for (; i<convSize; ++i)
            cinput[i] = 0.f;

        fft.fft(cinput.data());

        fft.complexMultiply(scratch.data(), cinput.data(), ckernel.data());

        fft.ifft(scratch.data());

        for (size_t i=0; i<tileSize+kernel.size(); ++i)
            if (tile*tileSize+i < conv.size())
                conv[tile * tileSize + i] += scratch[i];
    }
}

// convolve by splitting input & kernel
template <typename F>
void convolve3(std::vector<F>& conv,
               const std::vector<F>& input,
               const std::vector<F>& kernel)
{
    const size_t blockSize = 384;
    const size_t convSize = nextPowerOfTwo(blockSize*2);
    const size_t numKernels = kernel.size() / blockSize;
    const size_t numBlocks = input.size() / blockSize;

    conv.clear(); conv.resize(input.size() + kernel.size(), F(0));

    MATH::OouraFFT<F> fft;
    fft.init(convSize);

    std::vector<F>
            scratch(convSize, F(0)),
            cinput(convSize, F(0)),
            ckernel(convSize, F(0));

    size_t ipos = 0;
    for (size_t inp=0; inp < numBlocks; ++inp)
    {
        // prepare ft input
        size_t i;
        for (i=0; i<blockSize; ++i)
            cinput[i] = ipos < input.size() ? input[ipos++] : F(0);
        for (; i<convSize; ++i)
            cinput[i] = F(0);
        fft.fft(cinput.data());

        size_t kpos = 0;
        for (size_t k=0; k<numKernels; ++k)
        {
            // prepare ft kernel
            size_t i;
            for (i=0; i<blockSize; ++i)
                ckernel[i] = kpos < kernel.size() ? kernel[kpos++] : F(0);
            for (; i<ckernel.size(); ++i)
                ckernel[i] = F(0);
            fft.fft(ckernel.data());

            // convolve
            fft.complexMultiply(scratch.data(), cinput.data(), ckernel.data());
            fft.ifft(scratch.data());

            // mix into output
            for (size_t i=0; i<convSize; ++i)
            {
                const size_t outPos = inp * blockSize + k * blockSize + i;
                if (outPos < conv.size())
                    conv[outPos] += scratch[i];
            }
        }
    }
}

// convolve via AUDIO::ConvolveBuffer
template <typename F>
void convolve4(std::vector<F>& conv,
               const std::vector<F>& input2,
               const std::vector<F>& kernel)
{
    AUDIO::AudioBuffer in(384), out(384);
    AUDIO::ConvolveBuffer c;

    auto input = input2;
    input.resize(input.size() + in.blockSize(), F(0));

    c.setKernel(kernel.data(), kernel.size());
    conv.clear(); conv.resize(input.size() + kernel.size() + in.blockSize(), F(0));

    size_t pos = 0;
    while (pos < conv.size() - in.blockSize())
    {
        if (pos < input.size() - in.blockSize())
            in.writeBlock(&input[pos]);
        else
            in.writeNullBlock();

        c.process(&in, &out);

        out.readBlock(&conv[pos]);

        pos += in.blockSize();
    }
}

} // namespace

void TestFft::runConvolutionDialog()
{
    const int sr = 1;

    std::vector<float>
            input(45055),
            kernel(16003),
            conv1, conv2, conv3, conv4;

    for (size_t i=0; i<input.size(); ++i)
    {
        float t = float(i)/44100.f;
        input[i] = .5 * AUDIO::Waveform::waveform(
                        t * (10.f+1000.f*(1.f-.5f*float(i)/input.size()))
                        , AUDIO::Waveform::T_SAW_DECAY)
                      * (.5+.4*AUDIO::Waveform::waveform(
                        t * 11.3f, AUDIO::Waveform::T_SQUARE));
    }

    for (size_t i=0; i<kernel.size(); ++i)
        //kernel[i] = .2*std::sin(float(i)/kernel.size() * 3.14159265);
        kernel[i] = .3f * (float(rand()) / RAND_MAX * 2.f - 1.f) / (1.f+.02f*i);

    for (size_t i=3000; i<kernel.size(); ++i)
        kernel[i] += -.5f * kernel[i-3000];

#if 0
    float sum = 0.00001f;
    for (size_t i=0; i<kernel.size(); ++i)
        sum += std::abs(kernel[i]);
    for (size_t i=0; i<kernel.size(); ++i)
        kernel[i] /= sum;
#endif

    TimeMessure tm;
    tm.start(); convolve1(conv1, input, kernel); auto e1 = tm.time();
    tm.start(); convolve2(conv2, input, kernel); auto e2 = tm.time();
    tm.start(); convolve3(conv3, input, kernel); auto e3 = tm.time();
#if 1
    tm.start(); convolve4(conv4, input, kernel); auto e4 = tm.time();
#else
    assert(conv2.size() == conv3.size());
    conv4.resize(conv2.size());
    for (size_t i=0;i<conv2.size();++i)
        conv4[i] = conv2[i] - conv3[i];
    double e4 = 0.;
#endif

    MO_PRINT("Timing whole=" << e1 << ", overlap-add=" << e2
             << ", both-split=" << e3 << ", ConvolveBuffer=" << e4);

    // --- display ---

    auto sf_input =  AUDIO::SoundFileManager::createSoundFile(1, sr),
         sf_kernel = AUDIO::SoundFileManager::createSoundFile(1, sr),
         sf_conv1 =  AUDIO::SoundFileManager::createSoundFile(1, sr),
         sf_conv2 =  AUDIO::SoundFileManager::createSoundFile(1, sr),
         sf_conv3 =  AUDIO::SoundFileManager::createSoundFile(1, sr),
         sf_conv4 =  AUDIO::SoundFileManager::createSoundFile(1, sr);

    assert(sf_input->isWriteable());
    assert(sf_input->isOk());

    sf_input->appendDeviceData(input.data(), input.size());
    sf_kernel->appendDeviceData(kernel.data(), kernel.size());
    sf_conv1->appendDeviceData(conv1.data(), conv1.size());
    sf_conv2->appendDeviceData(conv2.data(), conv2.size());
    sf_conv3->appendDeviceData(conv3.data(), conv3.size());
    sf_conv4->appendDeviceData(conv4.data(), conv4.size());

    MO_PRINT(     "input  : " << sf_input->infoString()
             << "\nkernel : " << sf_kernel->infoString()
             << "\nconv1  : " << sf_conv1->infoString()
             << "\nconv2  : " << sf_conv2->infoString()
             << "\nconv3  : " << sf_conv3->infoString()
             << "\nconv4  : " << sf_conv4->infoString()
             );

    auto diag = new QDialog(application()->mainWindow());
    diag->setWindowTitle("Convolution test");
    diag->setAttribute(Qt::WA_DeleteOnClose, true);

    auto lv = new QVBoxLayout(diag);

        auto w = new GUI::SoundFileWidget(diag);
        lv->addWidget(w);

        QObject::connect(w, &GUI::SoundFileWidget::doubleClicked,
                         [](AUDIO::SoundFile* sf, Double)
        {
            AUDIO::AudioPlayer::play(sf);
        });

        w->setSoundFile(sf_kernel, 0); sf_kernel->release();
        w->setSoundFile(sf_input, 1);  sf_input->release();
        w->setSoundFile(sf_conv1, 2);  sf_conv1->release();
        w->setSoundFile(sf_conv2, 3);  sf_conv2->release();
        w->setSoundFile(sf_conv3, 4);  sf_conv3->release();
        w->setSoundFile(sf_conv4, 5);  sf_conv4->release();

    diag->show();
}




} // namespace MO
