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

#define OLD_METHOD

namespace MO {
namespace AUDIO {

struct ConvolveBuffer::Private
{
    Private(ConvolveBuffer*p)
        : p         (p)
    #ifdef OLD_METHOD
        , curBlockSize  (0)
    #endif
        , kernelChanged (true)
        , amplitude            (1.f)
    { }

    ~Private()
    {
    }

    void prepareComplexKernel(size_t blockSize);


    ConvolveBuffer* p;

#ifdef OLD_METHOD
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
    F32 kernelSum;
#else
    void push(const AudioBuffer* in);

    std::vector<F32>
            kernel, kernel_fft,
    scratch, output;
size_t chunkSize, numChunks, halfSize, curOutChunk;

    AUDIO::ResampleBuffer<F32> inRebuf, outRebuf;
#endif

    bool kernelChanged;
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

#ifdef OLD_METHOD

void ConvolveBuffer::process(const AudioBuffer *in, AudioBuffer *out)
{
    p_->process(in, out);
}

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

        MO_PRINT("window " << j << ": " << k << ", sum=" << s);

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

void ConvolveBuffer::push(const AudioBuffer* in)
{
    p_->push(in);
}

bool ConvolveBuffer::pop(AudioBuffer* out)
{
    if (p_->outRebuf.size() != out->blockSize())
        p_->outRebuf.setSize(out->blockSize());

    return p_->outRebuf.pop(out->writePointer());
}

void ConvolveBuffer::Private::prepareComplexKernel(size_t blockSize)
{
    chunkSize = 1024;

    // half length of kernel in freq domain
    kernel_fft.resize( nextPowerOfTwo(chunkSize + kernel.size()) * 2 );

    numChunks = (kernel_fft.size()+chunkSize-1) / chunkSize;

    // copy and f-transform kernel
    size_t i;
    for (i=0; i<kernel.size(); ++i)
        kernel_fft[i] = kernel[i];
    for (; i<kernel_fft.size(); ++i)
        kernel_fft[i] = 0.f;

    // to fourier
    MATH::real_fft(&kernel_fft[0], kernel_fft.size());

    /*for (size_t i=0; i<kernel_fft.size(); ++i)
        std::cout << kernel_fft[i] << ", ";
    std::cout << std::endl;
    */

    // buffer space
    scratch.resize(kernel_fft.size());
    inRebuf.setSize(chunkSize);
    output.resize(numChunks * chunkSize);
    curOutChunk = 0;

    // acknowledge
    kernelChanged = false;
}

void ConvolveBuffer::Private::push(const AudioBuffer *in)
{
    // update complex kernel if necessary
    if (kernelChanged)
        prepareComplexKernel(in->blockSize());

    // get input block
    inRebuf.push(in->readPointer(), in->blockSize());

    while (inRebuf.pop(&scratch[0]))
    {
        // zero-pad input
        for (size_t i=chunkSize; i<kernel_fft.size(); ++i)
            scratch[i] = 0.f;

        // f-transform
        MATH::real_fft(&scratch[0], kernel_fft.size());

        MATH::complex_multiply(
                    &scratch[0],
                    &scratch[0], &kernel_fft[0],
                    kernel_fft.size());

        // t-transform
        MATH::ifft(&scratch[0], kernel_fft.size());

        // copy/mix to output
        const F32* src = &scratch[0];
        F32* out = &output[curOutChunk * halfSize];
        for (size_t j=0; j<numChunks; ++j)
        {
            if (j+1 < numChunks)
                for (size_t i=0; i<chunkSize; ++i)
                    *out++ += *src++ * amplitude;
            else
                for (size_t i=0; i<chunkSize; ++i)
                    *out++ = *src++ * amplitude;

            outRebuf.push(&output[curOutChunk*chunkSize], chunkSize);
            curOutChunk++;
            if (curOutChunk >= numChunks)
                curOutChunk = 0;
        }

        //outRebuf.push(&output[(1-curOutChunk) * halfSize], halfSize);
    }
}

#endif

} // namespace AUDIO
} // namespace DELAY
