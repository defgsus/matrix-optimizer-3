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


const QStringList MultiFilter::filterTypeIds =
{ "low", "high", "band" };

const QStringList MultiFilter::filterTypeNames =
{ QObject::tr("1st order low-pass"),
  QObject::tr("1st order high-pass"),
  QObject::tr("1st order band-pass") };

const QStringList MultiFilter::filterTypeStatusTips =
{ QObject::tr("1st order low-pass"),
  QObject::tr("1st order high-pass"),
  QObject::tr("1st order band-pass") };

const QList<int> MultiFilter::filterTypeEnums =
{ T_FIRST_ORDER_LOW,
  T_FIRST_ORDER_HIGH,
  T_FIRST_ORDER_BAND };

MultiFilter::MultiFilter()
    : type_     (T_FIRST_ORDER_LOW),
      sr_       (44100),
      freq_     (1000),
      reso_     (0),
      s1_       (0),
      s2_       (0)

{
    updateCoefficients();
}

void MultiFilter::reset()
{
    s1_ = s2_ = 0;
}

void MultiFilter::updateCoefficients()
{
    MO_ASSERT(sr_ > 0, "samplerate f***ed up");

    // first order lowpass
    switch (type_)
    {
        case T_FIRST_ORDER_LOW:
        case T_FIRST_ORDER_HIGH:
        case T_FIRST_ORDER_BAND:
            q1_ = std::min(1.f, 2.f * freq_ / sr_);
        break;
    }
}

void MultiFilter::process(const F32 *input, uint inputStride,
                                F32 *output, uint outputStride, uint blockSize)
{
    switch (type_)
    {
        case T_FIRST_ORDER_LOW:
            for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
            {
                s1_ += q1_ * (*input - s1_);
                *output = s1_;
            }
        break;

        case T_FIRST_ORDER_HIGH:
            for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
            {
                s1_ += q1_ * (*input - s1_);
                // highpass == input minus lowpass
                *output = *input - s1_;
            }
        break;

        case T_FIRST_ORDER_BAND:
            for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
            {
                s1_ += q1_ * (*input - s1_);
                // lowpass of highpass
                s2_ += q1_ * ((*input - s1_) - s2_);
                *output = s2_;
            }
        break;
    }
}

} // namespace AUDIO
} // namespace MO
