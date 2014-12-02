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
            : p_sampleRate_(0),
              p_bufferSize_(0),
              p_numChannelsIn_(0),
              p_numChannelsOut_(0),
              p_sampleRateInv_(0.f)
        { }

        /** Constructor with initialization list */
        Configuration(uint sampleRate, uint bufferSize, uint channelsIn, uint channelsOut)
            : p_sampleRate_       (std::max(uint(1), sampleRate)),
              p_bufferSize_       (bufferSize),
              p_numChannelsIn_    (channelsIn),
              p_numChannelsOut_   (channelsOut),
              p_sampleRateInv_    (1.f / p_sampleRate_)
        { }

        // ---------- getter -----------

        bool isValid() const;

        uint sampleRate() const { return p_sampleRate_; }
        uint bufferSize() const { return p_bufferSize_; }
        uint numChannelsIn() const { return p_numChannelsIn_; }
        uint numChannelsOut() const { return p_numChannelsOut_; }
        F32  sampleRateInv() const { return p_sampleRateInv_; }

        // ---------- setter -----------

        void setSampleRate(uint sampleRate);
        void setBufferSize(uint bufferSize) { p_bufferSize_ = bufferSize; }
        void setNumChannelsIn(uint channels) { p_numChannelsIn_ = channels; }
        void setNumChannelsOut(uint channels) { p_numChannelsOut_ = channels; }

        // -------- comparison ---------

        bool operator == (const Configuration& c) const;

        bool operator != (const Configuration& c) const { return !(*this == c); }

    private:

        uint p_sampleRate_,
             p_bufferSize_,
             p_numChannelsIn_,
             p_numChannelsOut_;

        F32  p_sampleRateInv_;
    };


    inline bool Configuration::isValid() const
    {
        return p_sampleRate_ && p_bufferSize_ && (p_numChannelsIn_ || p_numChannelsOut_);
    }

    inline void Configuration::setSampleRate(uint sampleRate)
    {
        p_sampleRate_ = sampleRate;
        p_sampleRateInv_ = sampleRate? 1.f / sampleRate : 0.f;
    }

    inline bool Configuration::operator == (const Configuration& c) const
    {
        return p_sampleRate_ == c.p_sampleRate_
            && p_bufferSize_ == c.p_bufferSize_
            && p_numChannelsOut_ == c.p_numChannelsOut_
            && p_numChannelsIn_ == c.p_numChannelsIn_;
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
