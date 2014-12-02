/** @file audiobuffer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_AUDIOBUFFER_H
#define MOSRC_AUDIO_TOOL_AUDIOBUFFER_H

#include "types/float.h"

namespace MO {
namespace AUDIO {

class AudioBuffer
{
    public:

    // -------------- creation ------------------------

    AudioBuffer(size_t blockSize, size_t numBlocks = 1);

    void setSize(size_t blockSize, size_t numBlocks = 1);

    // ---------------- getter ------------------------

    size_t blockSize() const { return blockSize_; }
    size_t numBlocks() const { return numBlocks_; }

    // -------------- sampling ------------------------

    /** Returns a pointer to blockSize() floats to write to */
    F32 * insertPointer() { return &samples_[writeBlock_ * blockSize_]; }
    const F32 * insertPointer() const { return &samples_[writeBlock_ * blockSize_]; }

    /** Returns a read-pointer to the last written block */
    const F32 * readPointer() const { return &samples_[readBlock_ * blockSize_]; }

    /** Inserts one block of data into the buffer.
        @p block must point to at least blockSize() floats */
    void writeBlock(const F32 *block) { memcpy(insertPointer(), block, blockSize_ * sizeof(F32)); }

    /** Inserts one block of zeros into the buffer */
    void writeNullBlock() { memset(insertPointer(), 0, blockSize_ * sizeof(F32)); }

    /** Forwards the write pointer */
    void nextBlock()
    {
        // forward write pointer
        writeBlock_ = (writeBlock_ + 1) % numBlocks_;
        // forward read pointer
        readBlock_ = (readBlock_ + 1) % numBlocks_;
    }

    // ------ static convenience functions ----------

    /** Copies all input data to output data.
        If @p outputs has more channels that @p inputs,
        they will be zeroed.
        @note The blocksize must match between each input and output channel! */
    static void bypass(const QList<AUDIO::AudioBuffer *> &inputs,
                       const QList<AUDIO::AudioBuffer *> &outputs,
                       bool callNextBlock = false);


private:

    size_t blockSize_, numBlocks_, writeBlock_, readBlock_;
    std::vector<F32> samples_;
};




} // namespace AUDIO
} // namespace MO


#endif // AUDIOBUFFER_H
