/** @file convolvebuffer.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.05.2015</p>
*/

#include <cstdlib>

#include "convolvebuffer.h"
#include "audiobuffer.h"
#include "resamplebuffer.h"
#include "math/fft2.h"
#include "math/convolution.h"
#include "types/int.h"
#include "io/error.h"
#include "io/log.h"

#define MO_SPLIT_METHOD

namespace MO {
namespace AUDIO {

struct ConvolveBuffer::Private
{
    Private(ConvolveBuffer*p)
        : p         (p)
    #ifdef MO_SPLIT_METHOD
    #endif
        , curBlockSize  (0)
        , kernelChanged (true)
        , amplitude            (1.f)
    { }

    ~Private()
    {
    }

    void prepareComplexKernel(size_t blockSize);


    ConvolveBuffer* p;

#ifdef MO_SPLIT_METHOD
    void process(const AudioBuffer *in, AudioBuffer *out);
    std::vector<F32>
            kernel,
            scratch,
            input;
    std::vector<std::vector<F32>>
            kernel_fft;
    MATH::OouraFFT<F32> fft;
    AUDIO::AudioBuffer out_buf;
    size_t windowSize, numWindows, curBlockSize;
#else
    void process(const AudioBuffer *in, AudioBuffer *out);

    std::vector<F32>
            kernel, kernel_fft, input, scratch;
    size_t curBlockSize, numChunks;
    MATH::OouraFFT<F32> fft;
    AUDIO::AudioBuffer out_buf;
#endif

    bool kernelChanged;
    F32 kernelSum;
    F32 amplitude;

};


ConvolveBuffer::ConvolveBuffer()
    : p_            (new Private(this))
{
}

ConvolveBuffer::~ConvolveBuffer()
{
    delete p_;
}

void ConvolveBuffer::setKernel(const F32 *data, size_t num)
{
//    num = 384*2;//44100/4;

    p_->kernel.resize(num);
    memcpy(&p_->kernel[0], data, num * sizeof(F32));
    // XXX test spike
    //for (size_t i=0; i<num; ++i) p_->kernel[i] = i == 10 ? 1.f : 0.f;
    //for (size_t i=0; i<num; ++i) p_->kernel[i] = i == num-1 ? 1.f : 0.f;
    //for (size_t i=0; i<num; ++i) p_->kernel[i] = F32(rand()) / RAND_MAX * (1.f-F32(i)/num);

    p_->kernelChanged = true;
}

void ConvolveBuffer::setBlockSize(size_t blockSize)
{
    p_->prepareComplexKernel(blockSize);
}

void ConvolveBuffer::process(const AudioBuffer *in, AudioBuffer *out)
{
    p_->process(in, out);
}

#ifdef MO_SPLIT_METHOD

void ConvolveBuffer::Private::prepareComplexKernel(size_t blockSize)
{
    if (blockSize >= kernel.size())
        numWindows = 1;
    else
        numWindows = (kernel.size() + blockSize-1) / blockSize;
    windowSize = nextPowerOfTwo(blockSize * 2);
    curBlockSize = blockSize;
    fft.init(windowSize);

    MO_PRINT("ConvolveBuffer: kernel=" << kernel.size()
             << "; dspblock=" << curBlockSize
             << "; window=" << numWindows << "x" << windowSize
             << " = " << (numWindows * windowSize)
             << ", padding=" << (windowSize - (curBlockSize*2))
             );

    // copy, split and f-transform kernel
    kernel_fft.resize(numWindows);
    size_t k = 0;
    for (size_t j=0; j<numWindows; ++j)
    {
        kernel_fft[j].resize(windowSize);
        // split and pad
        size_t i;
        for (i=0; i<blockSize; ++i)
            kernel_fft[j][i] = k < kernel.size() ? kernel[k++] : 0.f;
        for (; i<windowSize; ++i)
            kernel_fft[j][i] = 0.f;

        F32 s = 0.f;
        for (size_t i = 0; i<windowSize; ++i)
            s += std::abs(kernel_fft[j][i]);

        //MO_PRINT("window " << j << ": " << k << ", sum=" << s);

        // to fourier
        fft.fft(kernel_fft[j].data());
    }

    // buffer space
    out_buf.setSize(blockSize, numWindows);
    scratch.resize(windowSize);
    input.resize(windowSize);

    // get global amp
    kernelSum = 0.0001f;
    for (size_t i=0; i<kernel.size(); ++i)
        kernelSum += std::abs(kernel[i]);

    amplitude = 1.f / kernelSum;

    // acknowledge
    curBlockSize = blockSize;
    kernelChanged = false;

}

void ConvolveBuffer::Private::process(const AudioBuffer *in, AudioBuffer *out)
{
    MO_ASSERT(in->blockSize() == out->blockSize(), "blocksize mismatch "
              << in->blockSize() << "/" << out->blockSize());

    // update complex kernel if necessary
    if (kernelChanged
        || in->blockSize() != curBlockSize)
        prepareComplexKernel(in->blockSize());

    // get input block
    in->readBlock(&input[0]);
    // pad
    for (size_t i=in->blockSize(); i<windowSize; ++i)
        input[i] = 0.f;

    // transform input
    fft.fft(input.data());

    // for each kernel window
    for (size_t k = 0; k < numWindows; ++k)
    {
        // multiply input with kernel window
        fft.complexMultiply(scratch.data(), input.data(), kernel_fft[k].data());

        // transform back
        fft.ifft(scratch.data());

        // add curBlockSize samples to output
        out_buf.writeAddBlock(scratch.data());
        // wraps around at numWindows
        out_buf.nextBlock();
        // add remaining samples to output
        out_buf.writeAddBlock(&scratch[curBlockSize], curBlockSize);
    }


    // copy to output
    out->writeBlock(out_buf.writePointer());

    // amplitude
    for (size_t i=0; i<out->blockSize(); ++i)
        out->writePointer()[i] *= 1.;//amplitude;

    // clear farmost future
    out_buf.previousBlock();
    out_buf.writeNullBlock();
    out_buf.nextBlock();
    out_buf.nextBlock();
}

#else


void ConvolveBuffer::Private::prepareComplexKernel(size_t blockSize)
{
    // half length of kernel in freq domain
    kernel_fft.resize( nextPowerOfTwo(blockSize + kernel.size()) );

    // copy and f-transform kernel
    size_t i;
    for (i=0; i<kernel.size(); ++i)
        kernel_fft[i] = kernel[i];
    for (; i<kernel_fft.size(); ++i)
        kernel_fft[i] = 0.f;

    // to fourier
    fft.init(kernel_fft.size());
    fft.fft(kernel_fft.data());

    // buffer space
    numChunks = (kernel_fft.size()+blockSize-1) / blockSize;
    input.resize(kernel_fft.size());
    scratch.resize(kernel_fft.size());
    out_buf.setSize(blockSize, numChunks);

    // get global amp
    kernelSum = 0.0001f;
    for (size_t i=0; i<kernel.size(); ++i)
        kernelSum += std::abs(kernel[i]);

    amplitude = 1.f / kernelSum;

    // acknowledge
    curBlockSize = blockSize;
    kernelChanged = false;
}


void ConvolveBuffer::Private::process(const AudioBuffer *in, AudioBuffer *out)
{
    MO_ASSERT(in->blockSize() == out->blockSize(), "blocksize mismatch "
              << in->blockSize() << "/" << out->blockSize());

    // update complex kernel if necessary
    if (kernelChanged
        || in->blockSize() != curBlockSize)
        prepareComplexKernel(in->blockSize());

    // get input block
    in->readBlock(&input[0]);
    // pad
    for (size_t i=in->blockSize(); i<kernel_fft.size(); ++i)
        input[i] = 0.f;

    // transform input
    fft.fft(input.data());

    // multiply input with kernel window
    fft.complexMultiply(scratch.data(), input.data(), kernel_fft.data());

    // transform back
    fft.ifft(scratch.data());

    // for each kernel window
    for (size_t k = 0; k < numChunks; ++k)
    {
        // wraps around at numChunks
        out_buf.nextBlock();

        // add curBlockSize samples to output
        out_buf.writeAddBlock(&scratch[k * curBlockSize]);
    }


    // copy to output
    out->writeBlock(out_buf.readPointer());

    // amplitude
    for (size_t i=0; i<out->blockSize(); ++i)
        out->writePointer()[i] *= 1.;//amplitude;

    // clear farmost future
    out_buf.previousBlock();
    out_buf.writeNullBlock();
    out_buf.nextBlock();
    out_buf.nextBlock();
}


#endif

} // namespace AUDIO
} // namespace DELAY
