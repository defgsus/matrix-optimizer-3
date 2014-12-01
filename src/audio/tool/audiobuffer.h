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

    /** Inserts one block of data into the buffer and forwards write pointer.
        @p block must point to at least blockSize() floats */
    void insert(const F32 *block);

    void insertNullBlock();

    /** Forwards the write pointer */
    void nextBlock();

private:

    size_t blockSize_, numBlocks_, writeBlock_, readBlock_;
    std::vector<F32> samples_;
};




} // namespace AUDIO
} // namespace MO


#endif // AUDIOBUFFER_H
