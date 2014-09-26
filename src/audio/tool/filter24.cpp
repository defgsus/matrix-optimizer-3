/** @file filter24.cpp

    @brief 24db/oct filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>

    adapted from: http://musicdsp.org/archive.php?classid=3#196 (azertopia@free.fr)
*/

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
    const F32 denorm = 0.00000001;

    for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
    {
        F32 x = *input - rc_ * y4_;

        y1_ = fc_ * x   + fc_ * oldx_  - fc2_ * y1_;
        y2_ = fc_ * y1_ + fc_ * oldy1_ - fc2_ * y2_;
        y3_ = fc_ * y2_ + fc_ * oldy2_ - fc2_ * y3_;
        y4_ = fc_ * y3_ + fc_ * oldy3_ - fc2_ * y4_;
        y4_ -= y4_ * y4_ * y4_ / 6.0;

        oldx_ = x;
        oldy1_ = y1_ + denorm;
        oldy2_ = y2_ + denorm;
        oldy3_ = y3_ + denorm;
        oldy4_ = y4_;

        *output = y4_;
    }
/*
  f := (Frq+Frq) / SR;
  p:=f*(1.8-0.8*f);
  k:=p+p-1.0;
  t:=(1.0-p)*1.386249;
  t2:=12.0+t*t;
  r := res*(t2+6.0*t)/(t2-6.0*t);

  x := inp - r*y4;

  y1:=x*p + oldx*p - k*y1;
  y2:=y1*p+oldy1*p - k*y2;
  y3:=y2*p+oldy2*p - k*y3;
  y4:=y3*p+oldy3*p - k*y4;
  y4 := y4 - ((y4*y4*y4)/6.0);
  oldx := x;
  oldy1 := y1+_kd;
  oldy2 := y2+_kd;;
  oldy3 := y3+_kd;;
  outlp := y4;
    */
}

} // namespace AUDIO
} // namespace MO


