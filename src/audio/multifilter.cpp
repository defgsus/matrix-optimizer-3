/** @file multifilter.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "multifilter.h"
#include "io/error.h"

namespace MO {
namespace AUDIO {


MultiFilter::MultiFilter()
    : sr_       (44100),
      freq_     (1000),
      reso_     (0),
      q1_       (0),
      s1_       (0)

{
    updateCoefficients();
}

void MultiFilter::reset()
{

}

void MultiFilter::updateCoefficients()
{
    MO_ASSERT(sr_ > 0, "samplerate f***ed up");

    // first order lowpass
    q1_ = std::min(1.f, 2.f * freq_ / sr_);
}

void MultiFilter::process(const F32 *input, uint inputStride,
                                F32 *output, uint outputStride, uint blockSize)
{
    // first order lowpass
    for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
    {
        s1_ += q1_ * (*input - s1_);
        *output = s1_;
    }
}

} // namespace AUDIO
} // namespace MO
