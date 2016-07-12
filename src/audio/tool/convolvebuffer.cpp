/** @file convolvebuffer.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.05.2015</p>
*/

#include <cstdlib>

#include "convolvebuffer.h"
#include "audiobuffer.h"
#include "math/fft.h"
#include "math/convolution.h"
#include "types/int.h"
#include "io/error.h"

namespace MO {
namespace AUDIO {

struct ConvolveBuffer::Private
{
    Private(ConvolveBuffer*p)
        : p         (p)
        , out_buf      (new AUDIO::AudioBuffer(1, 1))
        //: p_input_buf_      (new AUDIO::AudioBuffer(1, 1))
        , pos            (0)
        , curBlockSize  (0)
        , kernelChanged (true)
        , amplitude            (1.f)
    { }

    ~Private()
    {
        //delete p_input_buf_;
        delete out_buf;
    }

    void prepareComplexKernel(size_t blockSize);
    void process(const AudioBuffer *in, AudioBuffer *out);

    ConvolveBuffer* p;

#if 0 // naive version (too slow)
    std::vector<F32>
            p_kernel_,
            p_kernel_fft_,
            p_scratch_;
    AUDIO::AudioBuffer * p_input_buf_;
    size_t p_pos_, p_cur_blockSize_;
    bool p_kernel_changed_;
    F32 p_amp_;

#else

    std::vector<F32>
            kernel,
            scratch,
            input;
    std::vector<std::vector<F32>>
            kernel_fft;
    AUDIO::AudioBuffer * out_buf;
    size_t windowSize, windows, pos, curBlockSize;
    bool kernelChanged;
    F32 amplitude;

#endif

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
    p_->kernel.resize(num);
    memcpy(&p_->kernel[0], data, num * sizeof(F32));
    // XXX test, spike at 1 sec
    //for (size_t i=0; i<num; ++i) p_kernel_[i] = i == 0 ? 1.f : 0.f;

    // get global amp
    F32 sum = 0.000001f;
    for (size_t i=0; i<p_->kernel.size(); ++i)
        sum += std::abs(p_->kernel[i]);
    p_->amplitude = p_->kernel.size() / sum;
    MO_PRINT("sum = " << sum << "; amp = " << p_->amplitude);

    p_->kernelChanged = true;
}

#if 0

void ConvolveBuffer::prepareComplexKernel(size_t blockSize)
{
    // length of kernel in freq domain
    // also length of whole fft window
    size_t cnum = nextPowerOfTwo(p_kernel_.size() + blockSize) * 2;

    // copy kernel
    p_kernel_fft_.resize(cnum);
    for (auto & f : p_kernel_fft_)
        f = 0.f;
    for (size_t i=0; i<p_kernel_.size(); ++i)
        p_kernel_fft_[i] = p_kernel_[i];

    // transform
    MATH::real_fft(&p_kernel_fft_[0], cnum);

    // get amp
    Float ma = 0.00000001;
    for (size_t i=0; i<cnum; ++i)
        ma = std::max(ma, p_kernel_fft_[i]);
    p_amp_ = std::sqrt(F32(p_kernel_.size())) / ma;

    // buffer space
    p_scratch_.resize(cnum);

    // create an input buffer to hold enough history
    p_input_buf_->setSize(blockSize, cnum / blockSize + 1);
    MO_PRINT(p_input_buf_->blockSize() << " " << p_input_buf_->numBlocks());

    // acknowledge
    p_cur_blockSize_ = blockSize;
    p_kernel_changed_ = false;

}

void ConvolveBuffer::process(const AudioBuffer *in, AudioBuffer *out)
{
    MO_ASSERT(in->blockSize() == out->blockSize(), "blocksize mismatch "
              << in->blockSize() << "/" << out->blockSize());

    // update complex kernel if necessary
    if (p_kernel_changed_
        || in->blockSize() != p_cur_blockSize_)
        prepareComplexKernel(in->blockSize());

    // sample input
    p_input_buf_->writeBlock(in->readPointer());
    p_input_buf_->nextBlock();

    // copy history to scratch space
    p_input_buf_->readBlockLength(&p_scratch_[0], p_scratch_.size());

    // transform
    MATH::real_fft(&p_scratch_[0], p_scratch_.size());

    // multiply with kernel
    MATH::complex_multiply(&p_scratch_[0],
                           &p_scratch_[0], &p_kernel_fft_[0], p_scratch_.size());

    // transform back
    MATH::ifft(&p_scratch_[0], p_scratch_.size());

    // copy to output
    out->writeBlock(&p_scratch_[p_scratch_.size() - 1 - in->blockSize()]);
    //out->writeBlock(&p_scratch_[p_cur_blockSize_]);
    //out->writeBlock(&p_kernel_fft_[0]);

    for (size_t i=0; i<out->blockSize(); ++i)
        out->writePointer()[i] *= p_amp_;
}

#else

/*


*/


void ConvolveBuffer::prepareComplexKernel(size_t blockSize)
{
    p_->prepareComplexKernel(blockSize);
}

void ConvolveBuffer::process(const AudioBuffer *in, AudioBuffer *out)
{
    p_->process(in, out);
}

void ConvolveBuffer::Private::prepareComplexKernel(size_t blockSize)
{
    // length of kernel in freq domain
    size_t cnum = nextPowerOfTwo(std::max(kernel.size(), blockSize)) * 2;
    // length of single convolution window
    windowSize = nextPowerOfTwo(blockSize) * 2;
    // number of windows
    windows = std::max(size_t(1), cnum / windowSize);

    MO_PRINT("ConvolveBuffer: kernel=" << cnum << "(" << kernel.size() << ")"
             << "; dspblock=" << blockSize
             << "; window=" << windows << "x" << windowSize )

    // copy, split and f-transform kernel
    kernel_fft.resize(windows);
    size_t k = 0;
    for (size_t j=0; j<windows; ++j)
    {
        kernel_fft[j].resize(windowSize);
        // split and pad
        size_t i;
        for (i=0; i<windowSize/2; ++i)
            kernel_fft[j][i] = k < kernel.size() ? kernel[k++] : 0.f;
        for (; i<windowSize; ++i)
            kernel_fft[j][i] = 0.f;

        // to fourier
        MATH::real_fft(&kernel_fft[j][0], windowSize);
    }

    // buffer space
    scratch.resize(windowSize);
    input.resize(windowSize);

    // create an output buffer to hold 'the future'
    out_buf->setSize(blockSize, windows);

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
    MATH::real_fft(&input[0], windowSize);

    // for each kernel window
    for (size_t k = 0; k < windows; ++k)
    {
        // multiply with input with kernel window
        MATH::complex_multiply(&scratch[0],
                               &input[0], &kernel_fft[k][0], windowSize);

        // transform back
        MATH::ifft(&scratch[0], windowSize);

        // add to future output
        out_buf->writeAddBlock(&scratch[0]);
        // wraps around at p_windows_
        out_buf->nextBlock();
        // half window to next future output
        out_buf->writeAddBlock(&scratch[out->blockSize()]);
    }

    // copy to output
    out->writeBlock(out_buf->readPointer());

    // amplitude
    for (size_t i=0; i<out->blockSize(); ++i)
        out->writePointer()[i] *= amplitude;

    // clear farmost future
    out_buf->previousBlock();
    out_buf->writeNullBlock();
    out_buf->nextBlock();
    out_buf->nextBlock();
}

#endif

} // namespace AUDIO
} // namespace DELAY
