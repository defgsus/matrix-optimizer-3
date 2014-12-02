/** @file configuration.h

    @brief Basic audio-related configuration for devices and units

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#ifndef MOSRC_AUDIO_CONFIGURATION_H
#define MOSRC_AUDIO_CONFIGURATION_H

#include <iostream>

#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {

    /** Basic audio-related configuration for devices and units */
    class Configuration
    {
    public:

        /** Constructor creates an invalid Configuration */
        Configuration()
            : sampleRate_(0),
              bufferSize_(0),
              numChannelsIn_(0),
              numChannelsOut_(0),
              sampleRateInv_(0.f)
        { }

        /** Constructor with initialization list */
        Configuration(uint sampleRate, uint bufferSize, uint channelsIn, uint channelsOut)
            : sampleRate_       (std::max(uint(1), sampleRate)),
              bufferSize_       (bufferSize),
              numChannelsIn_    (channelsIn),
              numChannelsOut_   (channelsOut),
              sampleRateInv_    (1.f / sampleRate_)
        { }

        // ---------- getter -----------

        bool isValid() const;

        uint sampleRate() const { return sampleRate_; }
        uint bufferSize() const { return bufferSize_; }
        uint numChannelsIn() const { return numChannelsIn_; }
        uint numChannelsOut() const { return numChannelsOut_; }
        F32  sampleRateInv() const { return sampleRateInv_; }

        // ---------- setter -----------

        void setSampleRate(uint sampleRate);
        void setBufferSize(uint bufferSize) { bufferSize_ = bufferSize; }
        void setNumChannelsIn(uint channels) { numChannelsIn_ = channels; }
        void setNumChannelsOut(uint channels) { numChannelsOut_ = channels; }

        // -------- comparison ---------

        bool operator == (const Configuration& c) const;

        bool operator != (const Configuration& c) const { return !(*this == c); }

    private:

        uint sampleRate_,
             bufferSize_,
             numChannelsIn_,
             numChannelsOut_;

        F32  sampleRateInv_;
    };


    inline bool Configuration::isValid() const
    {
        return sampleRate_ && bufferSize_ && (numChannelsIn_ || numChannelsOut_);
    }

    inline void Configuration::setSampleRate(uint sampleRate)
    {
        sampleRate_ = sampleRate;
        sampleRateInv_ = sampleRate? 1.f / sampleRate : 0.f;
    }

    inline bool Configuration::operator == (const Configuration& c) const
    {
        return sampleRate_ == c.sampleRate_
            && bufferSize_ == c.bufferSize_
            && numChannelsOut_ == c.numChannelsOut_
            && numChannelsIn_ == c.numChannelsIn_;
    }

    template <typename T>
    inline std::basic_ostream<T>& operator << (std::basic_ostream<T>& out, const Configuration& conf)
    {
        out << "rate=" << conf.sampleRate() << ", block=" << conf.bufferSize()
            << "in=" << conf.numChannelsIn() << ", out=" << conf.numChannelsOut();
        return out;
    }

} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_CONFIGURATION_H
