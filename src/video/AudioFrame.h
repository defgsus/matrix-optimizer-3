/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 5/18/2016</p>
*/

#ifndef MCWSRC_MCW_AUDIOFRAME_H
#define MCWSRC_MCW_AUDIOFRAME_H

#include <vector>
#include <cstddef> // for size_t
#include <cstring> // for memcpy

#include "types/float.h"

using MO::F32;

//MCW_BEGIN_NAMESPACE


struct AudioFrame
{
public:

    /** Default constructor creates an invalid frame without memory */
    AudioFrame()
        : p_length_         (0)
        , p_channels_       (0)
        , p_sampleRate_     (0)
        , p_sampleRateInv_  (0.)
    { }

    /** Allocs memory for @p length * @p numChannels floats */
    AudioFrame(size_t length, size_t numChannels, size_t sampleRate,
               double presentationTime)
        : p_length_         (length)
        , p_channels_       (numChannels)
        , p_sampleRate_     (sampleRate)
        , p_sampleRateInv_  (sampleRate > 0 ? 1. / sampleRate : 0.)
        , p_pts_            (presentationTime)
        , p_data_           (p_length_ * p_channels_)
    { }

    // ------- getter ---------

    /** True if this frame contains data.
        A call to data() is illegal if the AudioFrame is not valid. */
    bool isValid() const { return !p_data_.empty(); }
    /** Number of frames, regardless of numChannels() */
    size_t length() const { return p_length_; }
    /** Length of frame in seconds */
    double lengthSeconds() const { return p_length_ * p_sampleRateInv_; }
    /** Number of channels */
    size_t numChannels() const { return p_channels_; }
    /** Number of floats -> length() * numChannels() */
    size_t sizeInSamples() const { return p_channels_ * p_length_; }
    /** Number of bytes -> length() * numChannels() * 4 */
    size_t sizeInBytes() const { return sizeInSamples() * 4; }
    /** Sampling rate in Hz */
    size_t sampleRate() const { return p_sampleRate_; }
    /** 1. / sampleRate() (division-by-zero-save) */
    double sampleRateInv() const { return p_sampleRateInv_; }

    /** Returns presentation time in seconds. */
    double presentationTime() const { return p_pts_; }

    /** Returns the pointer to length() * numChannels() consecutive floats.
        Illegal call if isValid() is false! */
    const F32* data() const { return &p_data_[0]; }

    // ---- setter ----

    /** Copies sizeInSamples() number of floats from @p d into internal data.
        Data in @p d is expected as interleaved channels. */
    void storeInterleavedData(const F32* d);

    /** Write access to length() * numChannels() consecutive floats.
        Illegal call if isValid() is false! */
    F32* data() { return &p_data_[0]; }

    /** Write access to the channel-interleaved consecutive floats at
        sample @p sample.
        Illegal call if isValid() is false! */
    F32* data(size_t sample) { return &p_data_[sample * p_channels_]; }

private:

    size_t
        p_length_,
        p_channels_,
        p_sampleRate_;
    double
        p_sampleRateInv_,
        p_pts_;
    std::vector<F32> p_data_;
};

// ------- IMPL. --------

inline void AudioFrame::storeInterleavedData(const F32* d)
{
    memcpy(static_cast<void*>(&p_data_[0]), static_cast<const void*>(d), sizeInBytes());
}


//MCW_END_NAMESPACE

#endif // MCWSRC_MCW_AUDIOFRAME_H

