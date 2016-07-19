/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#ifndef MOSRC_AUDIO_TOOL_FIXEDBLOCKDELAY
#define MOSRC_AUDIO_TOOL_FIXEDBLOCKDELAY

#include <list>
#include <vector>
#include <cstring>

#include "types/float.h"

#if 0
#   include "io/log.h"
#   define MO__D(arg__) MO_PRINT( \
        "FixedBlockDelay("<<this<<", "<<this->size() \
        <<", "<<this->delay()<<"): " << arg__)
#else
#   define MO__D(unused__) { }
#endif

namespace MO {
namespace AUDIO {

/** A delay for a fixed blocksize and delay-in-samples */
template <typename T>
class FixedBlockDelay
{
    public:

    // -------------- creation ------------------------

    FixedBlockDelay(size_t size = 1, size_t delay = 1);

    void setSize(size_t size) { setSize(size, p_delay_); }
    void setDelay(size_t delay) { setSize(p_size_, delay); }
    void setSize(size_t size, size_t delay);

    // ---------------- getter ------------------------

    size_t size() const { return p_size_; }
    size_t delay() const { return p_delay_; }

    // -------------- sampling ------------------------

    /** Writes a block of size() to the delay line */
    void write(const T* src);

    /** Reads a delayed block of size() into the @p dst buffer */
    void read(T* dst);

    /** A buffer of length size() that can be used as input or output */
    T* scratchBuffer() { return &p_scratch_[0]; }

    void writeFromScratchBuffer() { write(scratchBuffer()); }
    void readToScratchBuffer() { read(scratchBuffer()); }

    /** Zeroes buffer */
    void reset();

private:

    size_t p_size_, p_delay_,
            p_write_, p_read_;
    std::vector<T> p_buffer_, p_scratch_;
};


// ----------------------------------- templ. impl. ---------------------------------------------

template <typename T>
FixedBlockDelay<T>::FixedBlockDelay(size_t s, size_t d)
    : p_size_   (0)
    , p_delay_  (0)
    , p_write_  (0)
    , p_read_   (0)
{
    setSize(s, d);
}

template <typename T>
inline void FixedBlockDelay<T>::setSize(size_t s, size_t d)
{
    p_size_ = s;
    p_delay_ = d;
    p_buffer_.resize(s + d);
    p_scratch_.resize(s);
    reset();
}

template <typename T>
inline void FixedBlockDelay<T>::reset()
{
    p_write_ = p_delay_;
    p_read_ = 0;
    for (auto& f : p_buffer_)
        f = T(0);
    for (auto& f : p_scratch_)
        f = T(0);
}

template <typename T>
inline void FixedBlockDelay<T>::write(const T* src)
{
    MO__D("write() p_write_="<<p_write_<<", p_read_="<<p_read_);

    size_t w = 0;
    while (w < p_size_)
    {
        size_t num = std::min(p_size_ - w, p_buffer_.size()-p_write_);
        memcpy(&p_buffer_[p_write_], src + w, num * sizeof(T));
        p_write_ = (p_write_ + num) % p_buffer_.size();
        w += num;
    }

    MO__D("write() done, p_write_="<<p_write_<<", p_read_="<<p_read_);
}

template <typename T>
inline void FixedBlockDelay<T>::read(T* dst)
{
    MO__D("read() p_write_="<<p_write_<<", p_read_="<<p_read_);

    size_t w = 0;
    while (w < p_size_)
    {
        size_t num = std::min(p_size_ - w, p_buffer_.size() - p_read_);
        memcpy(dst + w, &p_buffer_[p_read_], num * sizeof(T));
        p_read_ = (p_read_+ num) % p_buffer_.size();
        w += num;
    }
}



} // namespace AUDIO
} // namespace MO

#undef MO__D

#endif // MOSRC_AUDIO_TOOL_FIXEDBLOCKDELAY

