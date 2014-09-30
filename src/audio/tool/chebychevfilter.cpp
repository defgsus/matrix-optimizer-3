/** @file chebychevfilter.cpp

    @brief 2nd order 24db/oct chebychev filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/12/2014</p>
*/

#include <cmath>

#include "chebychevfilter.h"
#include "io/error.h"
#include "math/constants.h"

namespace MO {
namespace AUDIO {


ChebychevFilter::ChebychevFilter()
    : type_     (T_LOWPASS),
      sr_       (44100),
      freq_     (1000),
      reso_     (0),
      clip_     (2)

{
    reset();
    updateCoefficients();
}

void ChebychevFilter::reset()
{
    stage0_ = stage1_ =
    state0_ = state1_ = state2_ = state3_ =
    bstage0_ = bstage1_ =
    bstate0_ = bstate1_ = bstate2_ = bstate3_ = 0;
}

void ChebychevFilter::updateCoefficients()
{
    MO_ASSERT(sr_ > 0, "samplerate f***ed up");

    F32     k = std::tan(PI * std::max((F32)2, freq_) / sr_),

            cg = std::min((F32)2, std::max((F32)0, (F32)2 - reso_)),
            sg = std::sinh(cg);

    cg = std::cosh(cg);
    cg *= cg;

    const F32
            coeff0 = (F32)1 / (cg - (F32)0.85355339059327376220042218105097),
            coeff1 = k * coeff0 * sg * (F32)1.847759065022573512256366378792,
            coeff2 = (F32)1 / (cg - (F32)0.14644660940672623779957781894758),
            coeff3 = k * coeff2 * sg * (F32)0.76536686473017954345691996806;

    k *= k*1.1185; // emperical value

    a0_ = (F32)1 / (coeff1 + k + coeff0);
    a1_ = (F32)2 * (coeff0 - k) * a0_;
    a2_ = (coeff1 - k - coeff0) * a0_;
    b0_ = a0_ * k;
    b1_ = (F32)2 * b0_;
    b2_ = b0_;

    a3_ = (F32)1 / (coeff3 + k + coeff2);
    a4_ = (F32)2 * (coeff2 - k) * a3_;
    a5_ = (coeff3 - k - coeff2) * a3_;
    b3_ = a3_ * k;
    b4_ = (F32)2 * b3_;
    b5_ = b3_;
}

void ChebychevFilter::process(const F32 *input, uint inputStride,
                                F32 *output, uint outputStride, uint blockSize)
{
#define MO__CLIP(v__) std::max(-clip_,std::min(clip_, (v__) ))

    if (type_ == T_LOWPASS)
    {
        for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
        {
            const F32 inp = MO__CLIP( *input );

            stage1_ = MO__CLIP( b0_ * inp + state0_ );
            state0_ = MO__CLIP( b1_ * inp + a1_ * stage1_ + state1_ );
            state1_ = MO__CLIP( b2_ * inp + a2_ * stage1_ );
            *output = MO__CLIP( b3_ * stage1_ + state2_ );
            state2_ = MO__CLIP( b4_ * stage1_ + a4_ * *output + state3_ );
            state3_ = MO__CLIP( b5_ * stage1_ + a5_ * *output );
        }
    }
    else if (type_ == T_HIGHPASS)
    {
        for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
        {
            const F32 inp = MO__CLIP( *input );

            stage1_ = MO__CLIP( b0_ * inp + state0_ );
            state0_ = MO__CLIP( b1_ * inp + a1_ * stage1_ + state1_ );
            state1_ = MO__CLIP( b2_ * inp + a2_ * stage1_ );
            stage0_ = MO__CLIP( b3_ * stage1_ + state2_ );
            state2_ = MO__CLIP( b4_ * stage1_ + a4_ * stage0_ + state3_ );
            state3_ = MO__CLIP( b5_ * stage1_ + a5_ * stage0_ );

            // high pass == signal minus low pass
            *output = inp - stage0_;
        }
    }
    else // bandpass
    {
        for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
        {
            F32 inp = MO__CLIP( *input );

            stage1_ = MO__CLIP( b0_ * inp + state0_ );
            state0_ = MO__CLIP( b1_ * inp + a1_ * stage1_ + state1_ );
            state1_ = MO__CLIP( b2_ * inp + a2_ * stage1_ );
            stage0_ = MO__CLIP( b3_ * stage1_ + state2_ );
            state2_ = MO__CLIP( b4_ * stage1_ + a4_ * stage0_ + state3_ );
            state3_ = MO__CLIP( b5_ * stage1_ + a5_ * stage0_ );

            // high pass == signal minus low pass
            inp -= stage0_ * F32(0.5);

            // now lowpass on top
            bstage1_ = MO__CLIP( b0_ * inp + bstate0_ );
            bstate0_ = MO__CLIP( b1_ * inp + a1_ * bstage1_ + bstate1_ );
            bstate1_ = MO__CLIP( b2_ * inp + a2_ * bstage1_ );
            *output  = MO__CLIP( (b3_ * bstage1_ + bstate2_) * F32(0.75) );
            bstate2_ = MO__CLIP( b4_ * bstage1_ + a4_ * *output + bstate3_ );
            bstate3_ = MO__CLIP( b5_ * bstage1_ + a5_ * *output );
        }
    }

#undef MO__CLIP
}

} // namespace AUDIO
} // namespace MO


