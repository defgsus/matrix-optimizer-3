/** @file filter24.cpp

    @brief 24db/oct filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>

    adapted from: http://musicdsp.org/archive.php?classid=3#196 (azertopia@free.fr)
*/

#include <limits>

#include "filter24.h"
#include "io/error.h"
#include "math/constants.h"

namespace MO {
namespace AUDIO {


Filter24::Filter24()
    : type_     (T_LOWPASS),
      sr_       (44100),
      freq_     (1000),
      reso_     (0)

{
    reset();
    updateCoefficients();
}

void Filter24::reset()
{
    y1_ = y2_ = y3_ = y4_ =
    oldx_ = oldy1_ = oldy2_ = oldy3_ = oldy4_ = 0.0;
    by1_ = by2_ = by3_ = by4_ =
    boldx_ = boldy1_ = boldy2_ = boldy3_ = boldy4_ = 0.0;
}

void Filter24::updateCoefficients()
{
    MO_ASSERT(sr_ > 0, "samplerate f***ed up");

    // freq coefficients

    F32 f = 2.0 * freq_ / sr_;

    fc_ = f * (1.8 - 0.8 * f);
    fc2_ = 2.0 * fc_ - 1.0;

    // resonance coefficients

    F32 t = (1.0 - fc_) * 1.386249,
        t2 = 12.0 + t * t;

    rc_ = reso_ * (t2 + 6.0 * t) / (t2 - 6.0 * t);
}

void Filter24::process(const F32 *input, uint inputStride,
                                F32 *output, uint outputStride, uint blockSize)
{
#define MO__LIMIT(v__) std::min(F32(1),std::max(F32(-1), (v__) ))

    const F32 denorm = std::numeric_limits<F32>::min();

    switch (type_)
    {
        case T_LOWPASS:
            for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
            {
                F32 x = *input - rc_ * y4_;

                y1_ = fc_ * x   + fc_ * oldx_  - fc2_ * y1_;
                y2_ = fc_ * y1_ + fc_ * oldy1_ - fc2_ * y2_;
                y3_ = fc_ * y2_ + fc_ * oldy2_ - fc2_ * y3_;
                y4_ = fc_ * y3_ + fc_ * oldy3_ - fc2_ * y4_;
                y4_ = MO__LIMIT( y4_ - y4_ * y4_ * y4_ / F32(6) );

                oldx_ = x;
                oldy1_ = y1_ + denorm;
                oldy2_ = y2_ + denorm;
                oldy3_ = y3_ + denorm;
                oldy4_ = y4_;

                *output = y4_;
            }
        break;

        case T_HIGHPASS:
            for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
            {
                F32 x = *input - rc_ * y4_;

                y1_ = fc_ * x   + fc_ * oldx_  - fc2_ * y1_;
                y2_ = fc_ * y1_ + fc_ * oldy1_ - fc2_ * y2_;
                y3_ = fc_ * y2_ + fc_ * oldy2_ - fc2_ * y3_;
                y4_ = fc_ * y3_ + fc_ * oldy3_ - fc2_ * y4_;
                y4_ = MO__LIMIT( y4_ - y4_ * y4_ * y4_ / F32(6) );

                oldx_ = x;
                oldy1_ = y1_ + denorm;
                oldy2_ = y2_ + denorm;
                oldy3_ = y3_ + denorm;
                oldy4_ = y4_;

                *output = *input - y4_;
            }
        break;

        case T_BANDPASS:
            for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
            {
                F32 x = *input - rc_ * y4_;

                y1_ = fc_ * x   + fc_ * oldx_  - fc2_ * y1_;
                y2_ = fc_ * y1_ + fc_ * oldy1_ - fc2_ * y2_;
                y3_ = fc_ * y2_ + fc_ * oldy2_ - fc2_ * y3_;
                y4_ = fc_ * y3_ + fc_ * oldy3_ - fc2_ * y4_;
                y4_ = MO__LIMIT( y4_ - y4_ * y4_ * y4_ / F32(6) );

                oldx_ = x;
                oldy1_ = y1_ + denorm;
                oldy2_ = y2_ + denorm;
                oldy3_ = y3_ + denorm;
                oldy4_ = y4_;

                // lowpass of highpass
                x = (*input - y4_) - rc_ * by4_;

                by1_ = fc_ * x    + fc_ * boldx_  - fc2_ * by1_;
                by2_ = fc_ * by1_ + fc_ * boldy1_ - fc2_ * by2_;
                by3_ = fc_ * by2_ + fc_ * boldy2_ - fc2_ * by3_;
                by4_ = fc_ * by3_ + fc_ * boldy3_ - fc2_ * by4_;
                by4_ = MO__LIMIT( by4_ - by4_ * by4_ * by4_ / F32(6) );

                boldx_ = x;
                boldy1_ = by1_ + denorm;
                boldy2_ = by2_ + denorm;
                boldy3_ = by3_ + denorm;
                boldy4_ = by4_;

                *output = by4_;
            }
        break;
    }



#undef MO__LIMIT
}

} // namespace AUDIO
} // namespace MO


