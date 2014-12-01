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
    F32 * insertPointer();
    const F32 * insertPointer() const;

    /** Inserts one block of data into the buffer.
        @p block must point to at least blockSize() floats */
    void insert(F32 * block);

private:

    size_t blockSize_, numBlocks_, writeBlock_;
    std::vector<F32> samples_;
};




} // namespace AUDIO
} // namespace MO


#endif // AUDIOBUFFER_H
