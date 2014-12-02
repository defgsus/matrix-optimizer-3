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

    size_t blockSize() const { return p_blockSize_; }
    size_t numBlocks() const { return p_numBlocks_; }

    // -------------- sampling ------------------------

    /** Returns a pointer to blockSize() floats to write to */
    F32 * insertPointer() { return &p_samples_[p_writeBlock_ * p_blockSize_]; }
    const F32 * insertPointer() const { return &p_samples_[p_writeBlock_ * p_blockSize_]; }

    /** Returns a read-pointer to the last written block */
    const F32 * readPointer() const { return &p_samples_[p_readBlock_ * p_blockSize_]; }

    /** Inserts one block of data into the buffer.
        @p block must point to at least blockSize() floats */
    void writeBlock(const F32 *block) { memcpy(insertPointer(), block, p_blockSize_ * sizeof(F32)); }

    /** Inserts one block of zeros into the buffer */
    void writeNullBlock() { memset(insertPointer(), 0, p_blockSize_ * sizeof(F32)); }

    /** Adds one block of data to the buffer.
        @p block must point to at least blockSize() floats. */
    void addBlock(const F32 * block) { auto p = insertPointer(); for (uint i=0; i < p_blockSize_; ++i) p[i] = block[i]; }

    /** Copies the current read-block into @p block */
    void readBlock(F32 * block) const { memcpy(block, readPointer(), p_blockSize_ * sizeof(F32)); }

    /** Forwards the write pointer */
    void nextBlock()
    {
        // forward write pointer
        p_writeBlock_ = (p_writeBlock_ + 1) % p_numBlocks_;
        // forward read pointer
        p_readBlock_ = (p_readBlock_ + 1) % p_numBlocks_;
    }

    // ------ static convenience functions ----------

    /** Copies all input data to output data.
        If @p outputs has more channels that @p inputs,
        they will be filled with zeros.
        Empty channels can be signaled with NULL and will be handled correctly.
        @note The blocksize must match between each input and output channel! */
    static void bypass(const QList<AUDIO::AudioBuffer *> &inputs,
                       const QList<AUDIO::AudioBuffer *> &outputs,
                       bool callNextBlock = false);

    /** Adds @p src on top of @p dst.
        Same convenience logic as bypass(). */
    static void mix(const QList<AUDIO::AudioBuffer *> &dst,
                    const QList<AUDIO::AudioBuffer *> &src,
                    bool callNextBlock = false);

private:

    size_t p_blockSize_, p_numBlocks_, p_writeBlock_, p_readBlock_;
    std::vector<F32> p_samples_;
};




} // namespace AUDIO
} // namespace MO


#endif // AUDIOBUFFER_H
