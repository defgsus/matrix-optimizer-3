/** @file envelopefollower.cpp

    @brief Envelope follower for audio signals

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "envelopefollower.h"
#include "io/error.h"

#include <cmath>

namespace MO {
namespace AUDIO {


EnvelopeFollower::EnvelopeFollower()
    : sr_       (44100),
      fadeIn_   (0.02),
      fadeOut_  (0.5),
      fadeAv_   (8.0),
      threshold_(0.0),
      ampIn_    (1.0),
      ampOut_   (1.0),
      env_      (0),
      av_       (0)

{
    updateCoefficients();
}

void EnvelopeFollower::reset()
{
    env_ = 0;
}

void EnvelopeFollower::updateCoefficients()
{
    MO_ASSERT(sr_ > 0, "samplerate f***ed up");

    qIn_ = 8.f / std::max(8.f, fadeIn_ * sr_);
    qOut_ = 8.f / std::max(8.f, fadeOut_ * sr_);
    qAv_ = 8.f / std::max(8.f, fadeAv_ * sr_);
}

F32 EnvelopeFollower::process(const F32 *input, uint inputStride, uint blockSize)
{
    for (uint i=0; i<blockSize; ++i, input += inputStride)
    {
        const F32 in = ampIn_ * std::abs(*input);

        // get average
        av_ += qAv_ * (in - av_);

        if (in >= 0.99)
            // XXX always indicate clipping
            env_ = 1;
        // raise?
        else if (in >= av_ + threshold_)
            env_ += qIn_ * (in - env_);
        // lower
        else
            env_ -= qOut_ * env_;
    }

    return env_ * ampOut_;
}

F32 EnvelopeFollower::process(const F32 *input, uint inputStride,
                              F32 *output, uint outputStride, uint blockSize)
{
    for (uint i=0; i<blockSize; ++i,
            input += inputStride,
            output += outputStride)
    {
        const F32 in = ampIn_ * std::abs(*input);

        // get average
        av_ += qAv_ * (in - av_);

        if (in >= 0.99)
            // XXX always indicate clipping
            env_ = 1;
        // raise?
        else if (in >= av_ + threshold_)
            env_ += qIn_ * (in - env_);
        // lower
        else
            env_ -= qOut_ * env_;

        *output = ampOut_ * env_;
    }

    return env_ * ampOut_;
}

} // namespace AUDIO
} // namespace MO
