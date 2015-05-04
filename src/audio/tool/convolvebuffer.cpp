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


ConvolveBuffer::ConvolveBuffer()
    : p_out_buf_      (new AUDIO::AudioBuffer(1, 1))
    //: p_input_buf_      (new AUDIO::AudioBuffer(1, 1))
    , p_pos_            (0)
    , p_cur_blockSize_  (0)
    , p_kernel_changed_ (true)
    , p_amp_            (1.f)
{
}

ConvolveBuffer::~ConvolveBuffer()
{
    //delete p_input_buf_;
    delete p_out_buf_;
}

void ConvolveBuffer::setKernel(const F32 *data, size_t num)
{
    p_kernel_.resize(num);
    memcpy(&p_kernel_[0], data, num * sizeof(F32));
    // XXX test, spike at 1 sec
    //for (size_t i=0; i<num; ++i) p_kernel_[i] = i == 44100 ? 1.f : 0.f;

    // get global amp
    Float sum = 0.0000001;
    for (size_t i=0; i<p_kernel_.size(); ++i)
        sum += std::abs(p_kernel_[i]);
    p_amp_ = p_kernel_.size() / sum;
    MO_PRINT("sum = " << sum << "; amp = " << p_amp_);

    p_kernel_changed_ = true;
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
    // length of kernel in freq domain
    size_t cnum = nextPowerOfTwo(std::max(p_kernel_.size(), blockSize)) * 2;
    // length of single convolution window
    p_windowSize_ = nextPowerOfTwo(blockSize) * 2;
    // number of windows
    p_windows_ = std::max(size_t(1), cnum / p_windowSize_);

    MO_PRINT("ConvolveBuffer: kernel=" << cnum << "(" << p_kernel_.size() << ")"
             << "; dspblock=" << blockSize
             << "; window=" << p_windows_ << "x" << p_windowSize_ )

    // copy, split and f-transform kernel
    p_kernel_fft_.resize(p_windows_);
    size_t k = 0;
    for (size_t j=0; j<p_windows_; ++j)
    {
        p_kernel_fft_[j].resize(p_windowSize_);
        // split and pad
        size_t i;
        for (i=0; i<p_windowSize_/2; ++i)
            p_kernel_fft_[j][i] = k < p_kernel_.size() ? p_kernel_[k++] : 0.f;
        for (; i<p_windowSize_; ++i)
            p_kernel_fft_[j][i] = 0.f;

        // to fourier
        MATH::real_fft(&p_kernel_fft_[j][0], p_windowSize_);
    }

    // buffer space
    p_scratch_.resize(p_windowSize_);
    p_input_.resize(p_windowSize_);

    // create an output buffer to hold 'the future'
    p_out_buf_->setSize(blockSize, p_windows_);

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

    // get input block
    in->readBlock(&p_input_[0]);
    // pad
    for (size_t i=in->blockSize(); i<p_windowSize_; ++i)
        p_input_[i] = 0.f;

    // transform input
    MATH::real_fft(&p_input_[0], p_windowSize_);

    // for each kernel window
    for (size_t k = 0; k < p_windows_; ++k)
    {
        // multiply with input with kernel window
        MATH::complex_multiply(&p_scratch_[0],
                               &p_input_[0], &p_kernel_fft_[k][0], p_windowSize_);

        // transform back
        MATH::ifft(&p_scratch_[0], p_windowSize_);

        // add to future output
        p_out_buf_->writeAddBlock(&p_scratch_[0]);
        // wraps around at p_windows_
        p_out_buf_->nextBlock();
        // half window to next future output
        p_out_buf_->writeAddBlock(&p_scratch_[out->blockSize()]);
    }

    // copy to output
    out->writeBlock(p_out_buf_->readPointer());

    // amplitude
    for (size_t i=0; i<out->blockSize(); ++i)
        out->writePointer()[i] *= p_amp_;

    // clear farmost future
    p_out_buf_->previousBlock();
    p_out_buf_->writeNullBlock();
    p_out_buf_->nextBlock();
    p_out_buf_->nextBlock();
}

#endif

} // namespace AUDIO
} // namespace DELAY
