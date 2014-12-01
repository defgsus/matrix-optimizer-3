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
      numBlocks_    (numBlocks),
      writeBlock_   (0),
      readBlock_    (0)
{
    setSize(blockSize_, numBlocks_);
}


void AudioBuffer::setSize(size_t blockSize, size_t numBlocks)
{
    MO_ASSERT(numBlocks > 0, "");

    blockSize_ = blockSize;
    numBlocks_ = numBlocks;
    readBlock_ = 0;
    writeBlock_ = 1 % numBlocks_;

    samples_.resize(blockSize, numBlocks);
    for (auto & s : samples_)
        s = 0;
}


void AudioBuffer::insert(const F32 *block)
{
    memcpy(insertPointer(), block, blockSize_ * sizeof(F32));
    nextBlock();
}

void AudioBuffer::insertNullBlock()
{
    memset(insertPointer(), 0, blockSize_ * sizeof(F32));
    nextBlock();
}

void AudioBuffer::nextBlock()
{
    // forward write pointer
    writeBlock_ = (writeBlock_ + 1) % numBlocks_;
    // forward read pointer
    readBlock_ = (readBlock_ + 1) % numBlocks_;
}



} // namespace AUDIO
} // namespace MO
