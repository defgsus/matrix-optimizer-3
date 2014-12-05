/** @file resamplebuffer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_RESAMPLEBUFFER_H
#define MOSRC_AUDIO_TOOL_ACCUMULATIONBUFFER_H

#include "types/float.h"

namespace MO {
namespace AUDIO {

/** Converts a stream of any blocksize to a constant blocksize */
template <typename T>
class ResampleBuffer
{
    public:

    // -------------- creation ------------------------

    ResampleBuffer(size_t blockSize = 1);

    void setSize(size_t blockSize);

    // ---------------- getter ------------------------

    size_t blockSize() const { return p_samples_.size(); }

    /** Returns the number of samples left to push with writeBlock()
        to receive blockSize() samples in readBlock(). */
    size_t left() const { return p_left_; }

    // -------------- sampling ------------------------

    /** Returns the sample at @p offset */
    T read(size_t offset) const;

    /** Returns a pointer to blockSize() number of samples */
    const T * readPointer() const { return &p_samples_[0]; }

    /** Inserts at max @p size samples of @p block into the buffer.
        The function stops, if blockSize() samples have been copied from @p block
        and returns the number of copied samples.
        The internal pointers are moved forward appropriately.
        @p block must point to at least @p size samples. */
    size_t writeBlock(const T *block, size_t size);

    /** Same as writeBlock() but sets all to zero. */
    size_t writeNullBlock(size_t size);

    /** Copies the current bufferSize() samples into @p block.
        left() will be equal to blockSize() after this call. */
    void readBlock(T * block);

private:

    size_t p_write_, p_read_, p_left_;
    std::vector<T> p_samples_;
};


// ----------------------------------- templ. impl. ---------------------------------------------

template <typename T>
ResampleBuffer<T>::ResampleBuffer(size_t s)
    : p_write_  (0),
      p_read_   (0),
      p_left_   (s)
{
    setSize(s);
}

template <typename T>
inline void ResampleBuffer<T>::setSize(size_t s)
{
    p_samples_.resize(std::max(size_t(1), s));
    for (auto & t : p_samples_)
        t = T(0);

    p_write_ = p_read_ = 0;
    p_left_ = s;
}

template <typename T>
inline T ResampleBuffer<T>::read(size_t offset) const
{
    return p_samples_[offset];
}

template <typename T>
inline size_t ResampleBuffer<T>::writeBlock(const T * block, size_t size)
{
    // take only as much as we need
    size_t num = std::min(size, p_left_);

    // copy data
    for (size_t i = 0; i < num; ++i)
        p_samples_[(p_write_ + i) % blockSize()] = block[i];

    // forward pointer
    p_write_ = (p_write_ + num) % blockSize();
    // remember how much we stored
    p_left_ -= num;

    return num;
}

template <typename T>
inline void ResampleBuffer<T>::readBlock(T * block)
{
    memcpy(block, &p_samples_[0], blockSize() * sizeof(T));

    // ready for more
    p_left_ = blockSize();
}




} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_ACCUMULATIONBUFFER_H
