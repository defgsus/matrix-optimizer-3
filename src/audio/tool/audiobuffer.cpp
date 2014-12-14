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
    : p_blockSize_    (0),
      p_numBlocks_    (0),
      p_writeBlock_   (0),
      p_readBlock_    (0)
{
    setSize(blockSize, numBlocks);
}


void AudioBuffer::setSize(size_t blockSize, size_t numBlocks)
{
    MO_ASSERT(numBlocks > 0, "");

    p_blockSize_ = blockSize;
    p_numBlocks_ = numBlocks;
    p_readBlock_ = 0;
    p_writeBlock_ = 1 % p_numBlocks_;

    p_samples_.resize(blockSize * numBlocks);
    for (auto & s : p_samples_)
        s = 0;
}

void AudioBuffer::writeBlock(const F32 *block, size_t stepsize)
{
    auto p = writePointer();
    for (size_t i = 0; i < blockSize(); ++i, block += stepsize, ++p)
        *p = *block;
}

void AudioBuffer::readBlock(F32 *block, size_t stepsize) const
{
    auto p = readPointer();
    for (size_t i = 0; i < blockSize(); ++i, block += stepsize, ++p)
        *block = *p;
}

void AudioBuffer::readBlockLength(F32 *block, size_t size) const
{
    MO_ASSERT(size < blockSize() * numBlocks(), size << " is out of range");

    size_t j = 0;
    size_t rp = p_readBlock_;
    auto p = readPointer();
    while (j < size)
    {
        // write current block
        for (size_t i = 0; i < blockSize() && j < size; ++i, ++j)
            block[j] = p[i];

        // go backwards in time
        rp = rp > 0 ? (rp - 1) : (p_numBlocks_ - 1);
        p = &p_samples_[p_readBlock_ * p_blockSize_];
    }

}

F32 AudioBuffer::readHistory(SamplePos history) const
{
    const SamplePos
            size = numBlocks() * blockSize();
    const SamplePosDiff
            read = (p_readBlock_ + 1) * blockSize() - 1;

    SamplePosDiff offset = read - std::min(history, size - 1);

    if (offset < 0)
        offset += size;

    return p_samples_[offset];
}




// ------------------------ static helper --------------------------------

void AudioBuffer::bypass(const QList<AUDIO::AudioBuffer *> &inputs,
                         const QList<AUDIO::AudioBuffer *> &outputs, bool callNextBlock)
{
    // copy inputs
    for (int i = 0; i<outputs.size(); ++i)
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


void AudioBuffer::mix(const QList<AUDIO::AudioBuffer *> &src,
                      const QList<AUDIO::AudioBuffer *> &dst, bool callNextBlock)
{
    for (int i = 0; i<dst.size(); ++i)
    if (dst[i])
    {
        if (i < src.size() && src[i] != 0)
        {
            MO_ASSERT(src[i]->blockSize() == dst[i]->blockSize(), "unmatched buffersize "
                      << src[i]->blockSize() << "/" << dst[i]->blockSize());

            dst[i]->writeAddBlock( src[i]->readPointer() );
        }
    }

    if (callNextBlock)
        for (auto o : dst)
            if (o)
                o->nextBlock();
}

void AudioBuffer::mix(const QList<AUDIO::AudioBuffer *> &src,
                      AUDIO::AudioBuffer * dst)
{
    for (int i = 0; i<src.size(); ++i)
    if (src[i])
    {
        MO_ASSERT(src[i]->blockSize() == dst->blockSize(), "unmatched buffersize "
                  << src[i]->blockSize() << "/" << dst->blockSize());

            dst->writeAddBlock( src[i]->readPointer() );
    }
}


void AudioBuffer::process(const QList<AudioBuffer *> &src, const QList<AudioBuffer *> &dst,
                          std::function<void (uint channel, const AudioBuffer *, AudioBuffer *)> func,
                          bool callNextBlock)
{
    for (int i = 0; i<dst.size(); ++i)
    if (dst[i])
    {
        if (i < src.size() && src[i] != 0)
        {
            MO_ASSERT(src[i]->blockSize() == dst[i]->blockSize(), "unmatched buffersize "
                      << src[i]->blockSize() << "/" << dst[i]->blockSize());

            func( i, src[i], dst[i] );
        }
        else
            dst[i]->writeNullBlock();
    }

    if (callNextBlock)
        for (auto o : dst)
            if (o)
                o->nextBlock();
}

} // namespace AUDIO
} // namespace MO
