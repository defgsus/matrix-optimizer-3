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


void AudioBuffer::bypass(const QList<AUDIO::AudioBuffer *> &inputs,
                         const QList<AUDIO::AudioBuffer *> &outputs, bool callNextBlock)
{
    const int num = std::min(inputs.size(), outputs.size());

    // copy inputs
    for (int i = 0; i<num; ++i)
    {
        MO_ASSERT(inputs[i]->blockSize() == outputs[i]->blockSize(), "unmatched buffersize "
                  << inputs[i]->blockSize() << "/" << outputs[i]->blockSize());
        outputs[i]->writeBlock( inputs[i]->readPointer() );
    }

    // clear remaining
    for (int i = num; i < outputs.size(); ++i)
    {
        outputs[i]->writeNullBlock();
    }

    if (callNextBlock)
        for (auto o : outputs)
            o->nextBlock();
}


} // namespace AUDIO
} // namespace MO
