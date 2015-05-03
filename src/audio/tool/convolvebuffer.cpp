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
    : p_input_buf_      (new AUDIO::AudioBuffer(1, 1))
    , p_pos_            (0)
    , p_cur_blockSize_  (0)
    , p_kernel_changed_ (true)
    , p_amp_            (1.f)
{
}

ConvolveBuffer::~ConvolveBuffer()
{
    delete p_input_buf_;
}

void ConvolveBuffer::setKernel(const F32 *data, size_t num)
{
    p_kernel_.resize(num);
    memcpy(&p_kernel_[0], data, num * sizeof(F32));
    p_kernel_changed_ = true;
}

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

} // namespace AUDIO
} // namespace DELAY
