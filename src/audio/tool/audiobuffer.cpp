/** @file audiobuffer.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include "audiobuffer.h"
#include "io/error.h"

namespace MO {
namespace AUDIO {

AudioBuffer::AudioBuffer(size_t blockSize, size_t numBlocks)
    : blockSize_    (blockSize),
      numBlocks_    (numBlocks)
{
    MO_ASSERT(numBlocks > 0, "");

    samples_.resize(blockSize, numBlocks);
    for (auto & s : samples_)
        s = 0;
}


void AudioBuffer::setSize(size_t blockSize, size_t numBlocks)
{
    MO_ASSERT(numBlocks > 0, "");

    samples_.resize(blockSize, numBlocks);
    for (auto & s : samples_)
        s = 0;
}

F32 * AudioBuffer::insertPointer()
{
    return &samples_[writeBlock_ * blockSize_];
}

const F32 * AudioBuffer::insertPointer() const
{
    return &samples_[writeBlock_ * blockSize_];
}

void AudioBuffer::insert(F32 *block)
{
    memcpy(insertPointer(), block, blockSize_ * sizeof(F32));
    nextWriteBlock();
}

void AudioBuffer::nextWriteBlock()
{
    // forward write pointer
    writeBlock_ = (writeBlock_ + 1) % numBlocks_;
}



} // namespace AUDIO
} // namespace MO
