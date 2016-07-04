/** @file resamplebuffer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_RESAMPLEBUFFER_H
#define MOSRC_AUDIO_TOOL_ACCUMULATIONBUFFER_H

#include <list>
#include <vector>
#include <cstring>

#include "types/float.h"

#if 0
#   include "io/log.h"
#   define MO__D(arg__) MO_PRINT("Resampler("<<this<<", "<<this->size()<<"): " << arg__)
#else
#   define MO__D(unused__) { }
#endif

namespace MO {
namespace AUDIO {

/** Converts a stream of any blocksize to a constant blocksize */
template <typename T>
class ResampleBuffer
{
    public:

    // -------------- creation ------------------------

    ResampleBuffer(size_t size = 1);

    void setSize(size_t size);

    // ---------------- getter ------------------------

    size_t size() const { return p_size_; }

    /** Returns the number of samples left to push with writeBlock()
        to receive blockSize() samples in readBlock(). */
    //size_t left() const { return p_left_; }

    // -------------- sampling ------------------------

    void clear();

    void push(const T* data, size_t size);

    bool pop(T* src);

private:

    size_t p_size_, p_avail_, p_filled_;
    std::list<std::vector<T>> p_list_;
};


// ----------------------------------- templ. impl. ---------------------------------------------

template <typename T>
ResampleBuffer<T>::ResampleBuffer(size_t s)
    : p_size_   (0)
    , p_avail_  (0)
    , p_filled_ (0)
{
    setSize(s);
}

template <typename T>
inline void ResampleBuffer<T>::setSize(size_t s)
{
    clear();
    p_size_ = s;
}

template <typename T>
inline void ResampleBuffer<T>::clear()
{
    p_list_.clear();
    p_avail_ = 0;
    p_filled_ = 0;
}

template <typename T>
inline void ResampleBuffer<T>::push(const T* src, size_t num)
{
    MO__D("push(" << num << ")");

    size_t written = 0;
    while (written < num)
    {
        if (p_list_.empty() || p_filled_ == p_size_)
        {
            MO__D("create new block " << p_size_);
            p_list_.push_front(std::vector<T>(p_size_));
            p_filled_ = 0;
        }
        T* dst = &p_list_.front()[0];

        size_t n = std::min(p_size_ - p_filled_, num - written);
        MO__D("copy " << n);
        memcpy(dst + p_filled_, src + written, n * sizeof(T));
        written += n;
        p_filled_ += n;
        p_avail_ += n;
    }
    MO__D("push() end, written="<<written<<", avail="<<p_avail_
      <<", filled="<<p_filled_<<", bufs="<<p_list_.size());
}

template <typename T>
inline bool ResampleBuffer<T>::pop(T* dst)
{
    MO__D("pop()");

    if (p_list_.empty())
        return false;
    if (p_list_.size() == 1 && p_filled_ < p_size_)
        return false;

    const std::vector<T>& vec = p_list_.back();
    memcpy(dst, &vec[0], p_size_ * sizeof(T));
    p_list_.pop_back();
    p_avail_ -= p_size_;
    MO__D("pop() done, avail=" << p_avail_ << ", bufs=" << p_list_.size());
    return true;
}



} // namespace AUDIO
} // namespace MO

#undef MO__D

#endif // MOSRC_AUDIO_TOOL_ACCUMULATIONBUFFER_H
