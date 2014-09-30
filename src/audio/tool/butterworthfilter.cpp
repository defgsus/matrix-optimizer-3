/** @file butterworthfilter.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.09.2014</p>
*/

#include <cmath>

#include "butterworthfilter.h"
#include "io/error.h"
#include "math/constants.h"
#include "io/log.h"

namespace MO {
namespace AUDIO {


ButterworthFilter::ButterworthFilter()
    : type_     (T_LOWPASS),
      sr_       (44100),
      freq_     (1000),
      reso_     (0),
      clip_     (4)

{
    reset();
    updateCoefficients();
}

void ButterworthFilter::reset()
{
    xm1_ = xm2_ = xm3_ = xm4_ =
    ym1_ = ym2_ = ym3_ = ym4_ = 0;
}

void ButterworthFilter::updateCoefficients()
{
    MO_ASSERT(sr_ > 0, "samplerate f***ed up");

    const F32
        fc = PI * freq_ / sr_,
        wc = 2.0 * fc,
        wc2 = wc * wc,
        wc3 = wc2 * wc,
        wc4 = wc2 * wc2,

        k = wc / std::tan(fc),
        k2 = k * k,
        k3 = k2 * k,
        k4 = k2 * k2,

        sqrt2 = std::sqrt(2.0),
        sq_tmp1 = sqrt2 * wc3 * k,
        sq_tmp2 = sqrt2 * wc * k3,
        a_tmp = 4.0 * wc2 * k2 + 2.0 * sq_tmp1 + k4 + 2.0 * sq_tmp2 + wc4;

    b1_ = (4.0 * (wc4 + sq_tmp1 - k4 - sq_tmp2)) / a_tmp;
    b2_ = (6.0 * wc4 - 8.0 * wc2 * k2 + 6.0 * k4) / a_tmp;
    b3_ = (4.0 * (wc4 - sq_tmp1 + sq_tmp2 - k4)) / a_tmp;
    b4_ = (k4 - 2.0 * sq_tmp1 + wc4 - 2 * sq_tmp2 + 4 * wc2 * k2) / a_tmp;

    switch (type_)
    {
        case T_LOWPASS:
            a0_ = wc4 / a_tmp;
            a1_ = 4.0 * wc4 / a_tmp;
            a2_ = 6.0 * wc4 / a_tmp;
            a3_ = a1_;
            a4_ = a0_;
        break;

        case T_HIGHPASS:
            a0_ = k4 / a_tmp;
            a1_ = -4.0 * k4 / a_tmp;
            a2_ = 6.0 * k4 / a_tmp;
            a3_ = a1_;
            a4_ = a0_;
        break;
    }

    const F32 reso = std::max(F32(0), std::min(F32(0.99999), reso_));
    q_ = reso
            // limit resonance at edges
            * std::pow(std::sin(fc*2), 2.5);

    // amplitude adjustment
    F32 af = std::pow(reso, F32(0.5));
    amp_ = F32(1) + af * F32(1./3 - 1.);

}

void ButterworthFilter::process(const F32 *input, uint inputStride,
                                F32 *output, uint outputStride, uint blockSize)
{
#define MO__CLIP(v__) std::max(-clip_,std::min(clip_, (v__) ))

    for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
    {
        const F32

            tempx = MO__CLIP( *input ),
            tempy = MO__CLIP(
                        a0_ * tempx
                      + a1_ * xm1_
                      + a2_ * xm2_
                      + a3_ * xm3_
                      + a4_ * xm4_
                      - b1_ * ym1_
                      - b2_ * ym2_
                      - b3_ * ym3_
                      - b4_ * ym4_
                    // hacked-in resonance
                      + q_ * (xm1_ - xm3_));

        xm4_ = xm3_;
        xm3_ = xm2_;
        xm2_ = xm1_;
        xm1_ = tempx;
        ym4_ = ym3_;
        ym3_ = ym2_;
        ym2_ = ym1_;
        ym1_ = tempy;

        *output = tempy * amp_;
    }

#undef MO__CLIP
}

} // namespace AUDIO
} // namespace MO
