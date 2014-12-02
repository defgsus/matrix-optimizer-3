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
    : p_blockSize_    (blockSize),
      p_numBlocks_    (numBlocks),
      p_writeBlock_   (0),
      p_readBlock_    (0)
{
    setSize(p_blockSize_, p_numBlocks_);
}


void AudioBuffer::setSize(size_t blockSize, size_t numBlocks)
{
    MO_ASSERT(numBlocks > 0, "");

    p_blockSize_ = blockSize;
    p_numBlocks_ = numBlocks;
    p_readBlock_ = 0;
    p_writeBlock_ = 1 % p_numBlocks_;

    p_samples_.resize(blockSize, numBlocks);
    for (auto & s : p_samples_)
        s = 0;
}

void AudioBuffer::bypass(const QList<AUDIO::AudioBuffer *> &inputs,
                         const QList<AUDIO::AudioBuffer *> &outputs, bool callNextBlock)
{
    const int num = std::max(inputs.size(), outputs.size());

    // copy inputs
    for (int i = 0; i<num; ++i)
    if (outputs[i])
    {
        // clear
        if (i >= inputs.size() || inputs[i] == 0)
            outputs[i]->writeNullBlock();
        // or copy
        else
        {
            MO_ASSERT(inputs[i]->blockSize() == outputs[i]->blockSize(), "unmatched buffersize "
                      << inputs[i]->blockSize() << "/" << outputs[i]->blockSize());
            outputs[i]->writeBlock( inputs[i]->readPointer() );
        }
    }

    if (callNextBlock)
        for (auto o : outputs)
            if (o)
                o->nextBlock();
}


void AudioBuffer::mix(const QList<AUDIO::AudioBuffer *> &dst,
                      const QList<AUDIO::AudioBuffer *> &src, bool callNextBlock)
{
    const int num = std::max(dst.size(), src.size());

    for (int i = 0; i<num; ++i)
    if (dst[i])
    {
        if (i < src.size() && src[i] != 0)
        {
            MO_ASSERT(src[i]->blockSize() == dst[i]->blockSize(), "unmatched buffersize "
                      << src[i]->blockSize() << "/" << dst[i]->blockSize());
            dst[i]->addBlock( src[i]->readPointer() );
        }
    }

    if (callNextBlock)
        for (auto o : dst)
            if (o)
                o->nextBlock();
}


} // namespace AUDIO
} // namespace MO
