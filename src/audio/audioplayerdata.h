/** @file audioplayerdata.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.05.2015</p>
*/

#ifndef MOSRC_AUDIO_AUDIOPLAYERDATA_H
#define MOSRC_AUDIO_AUDIOPLAYERDATA_H

#include "types/float.h"
#include "types/refcounted.h"

namespace MO {
namespace AUDIO {

/** Abstract data class to be played with AudioPlayer */
class AudioPlayerData : public RefCounted
{
public:
    AudioPlayerData() : RefCounted("AudioPlayerData") { }

    virtual size_t numChannels() const = 0;
    virtual size_t sampleRate() const = 0;
    virtual size_t lengthSamples() const = 0;
    Double lengthSeconds() const;
    /** [0, lengthSamples()) */
    virtual size_t pos() const = 0;
    /** lengthSamples() * lengthChannels(); */
    size_t size() const { return lengthSamples() * numChannels(); }

    virtual void seek(size_t pos) = 0;
    void seek(Double second);

    /** Fill @p buffer with @p blockSize * numChannels() data, non-interlaced. */
    virtual size_t get(F32 * buffer, size_t numChan, size_t blockSize) = 0;

    /** Fill @p buffer with @p blockSize * numChannels() data, interlaced.
        Default implementation returns interlaced data from get() */
    virtual size_t getInterlaced(F32 * buffer, size_t numChan, size_t blockSize);
};



class AudioPlayerSample : public AudioPlayerData
{
public:
    explicit AudioPlayerSample(size_t length, size_t numChannels, size_t sampleRate);

    /** Creates a new instance by copying the given data */
    static AudioPlayerSample * fromData(const F32 * data, size_t length, size_t sampleRate);

    virtual size_t numChannels() const { return p_chan_; }
    virtual size_t sampleRate() const { return p_rate_; }
    virtual size_t lengthSamples() const { return p_len_; }
    virtual size_t pos() const { return p_pos_; }

    virtual void seek(size_t pos) { p_pos_ = pos; }

    virtual size_t get(F32 * buffer, size_t numChan, size_t blockSize);
    virtual size_t getInterlaced(F32 * buffer, size_t numChan, size_t blockSize);

    F32 * samples() { return &p_data_[0]; }
    const F32 * samples() const { return &p_data_[0]; }

private:

    size_t p_len_, p_chan_, p_rate_, p_pos_;
    std::vector<F32> p_data_;
};




} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_AUDIOPLAYERDATA_H
