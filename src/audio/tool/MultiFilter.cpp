/** @file multifilter.cpp

    @brief Multi-mode audio filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include <cmath>

#include "MultiFilter.h"
#include "Filter24.h"
#include "ChebychevFilter.h"
#include "ButterworthFilter.h"
#include "FixedFilter.h"
#include "io/error.h"
#include "math/interpol.h"

namespace MO {
namespace AUDIO {


const QStringList MultiFilter::filterTypeIds =
{ "bypass", "low", "high", "band",
  "nlow", "nhigh", "nband",
  "low24", "high24", "band24",
  "cheblow", "chebhigh", "chebband",
  "butlow", "buthigh",
  "nbeslow", "nbeshigh", "nbesband",
  "nchebylow", "nchebyhigh", "nchebyband",
  "nbutlow", "nbuthigh", "nbutband"
};

const QStringList MultiFilter::filterTypeNames =
{ QObject::tr("off"),
  QObject::tr("1st order low-pass"),
  QObject::tr("1st order high-pass"),
  QObject::tr("1st order band-pass"),
  QObject::tr("nth order low-pass"),
  QObject::tr("nth order high-pass"),
  QObject::tr("nth order band-pass"),
  QObject::tr("24db/oct low-pass"),
  QObject::tr("24db/oct high-pass"),
  QObject::tr("24db/oct band-pass"),
  QObject::tr("2nd chebychev low"),
  QObject::tr("2nd chebychev high"),
  QObject::tr("2nd chebychev band"),
  QObject::tr("4th butterworth low"),
  QObject::tr("4th butterworth high"),
  QObject::tr("nth bessel low-pass"),
  QObject::tr("nth bessel high-pass"),
  QObject::tr("nth bessel band-pass"),
  QObject::tr("nth chebychev low-pass"),
  QObject::tr("nth chebychev high-pass"),
  QObject::tr("nth chebychev band-pass"),
  QObject::tr("nth butterworth low-pass"),
  QObject::tr("nth butterworth high-pass"),
  QObject::tr("nth butterworth band-pass")
};

const QStringList MultiFilter::filterTypeStatusTips =
{ QObject::tr("No filtering takes place"),
  QObject::tr("1st order, 6db/oct, low-pass"),
  QObject::tr("1st order, 6db/oct, high-pass"),
  QObject::tr("1st order, 6db/oct, band-pass"),
  QObject::tr("n repeated stages of 1st order, 6db/oct, low-pass"),
  QObject::tr("n repeated stages of 1st order, 6db/oct, high-pass"),
  QObject::tr("n repeated stages of 1st order, 6db/oct, band-pass"),
  QObject::tr("1st order, 24db/oct, low-pass"),
  QObject::tr("1st order, 24db/oct, high-pass"),
  QObject::tr("1st order, 24db/oct, band-pass"),
  QObject::tr("2nd order, 24db/oct, chebychev low-pass"),
  QObject::tr("2nd order, 24db/oct, chebychev high-pass"),
  QObject::tr("2nd order, 24db/oct, chebychev band-pass"),
  QObject::tr("4th order Linkwitz-Riley low-pass"),
  QObject::tr("4th order Linkwitz-Riley high-pass"),
  QObject::tr("nth order bessel low-pass"),
  QObject::tr("nth order bessel high-pass"),
  QObject::tr("nth order bessel band-pass"),
  QObject::tr("nth order chebychev low-pass"),
  QObject::tr("nth order chebychev high-pass"),
  QObject::tr("nth order chebychev band-pass"),
  QObject::tr("nth order butterworth low-pass"),
  QObject::tr("nth order butterworth high-pass"),
  QObject::tr("nth order butterworth band-pass")
};

const QList<int> MultiFilter::filterTypeEnums =
{ T_BYPASS,
  T_FIRST_ORDER_LOW,
  T_FIRST_ORDER_HIGH,
  T_FIRST_ORDER_BAND,
  T_NTH_ORDER_LOW,
  T_NTH_ORDER_HIGH,
  T_NTH_ORDER_BAND,
  T_24_LOW,
  T_24_HIGH,
  T_24_BAND,
  T_CHEBYCHEV_LOW,
  T_CHEBYCHEV_HIGH,
  T_CHEBYCHEV_BAND,
  T_BUTTERWORTH_LOW,
  T_BUTTERWORTH_HIGH,
  T_NTH_BESSEL_LOW,
  T_NTH_BESSEL_HIGH,
  T_NTH_BESSEL_BAND,
  T_NTH_CHEBYCHEV_LOW,
  T_NTH_CHEBYCHEV_HIGH,
  T_NTH_CHEBYCHEV_BAND,
  T_NTH_BUTTERWORTH_LOW,
  T_NTH_BUTTERWORTH_HIGH,
  T_NTH_BUTTERWORTH_BAND
};

bool MultiFilter::supportsOrder(FilterType t)
{
    return t == T_NTH_ORDER_LOW
        || t == T_NTH_ORDER_HIGH
        || t == T_NTH_ORDER_BAND
        || t == T_NTH_BESSEL_LOW
        || t == T_NTH_BESSEL_HIGH
        || t == T_NTH_BESSEL_BAND
        || t == T_NTH_CHEBYCHEV_LOW
        || t == T_NTH_CHEBYCHEV_HIGH
        || t == T_NTH_CHEBYCHEV_BAND
        || t == T_NTH_BUTTERWORTH_LOW
        || t == T_NTH_BUTTERWORTH_HIGH
        || t == T_NTH_BUTTERWORTH_BAND;
}

bool MultiFilter::supportsResonance(FilterType t)
{
    return !(
           t == T_NTH_BESSEL_LOW
        || t == T_NTH_BESSEL_HIGH
//        || t == T_NTH_BESSEL_BAND
        || t == T_NTH_CHEBYCHEV_LOW
        || t == T_NTH_CHEBYCHEV_HIGH
//        || t == T_NTH_CHEBYCHEV_BAND
        || t == T_NTH_BUTTERWORTH_LOW
        || t == T_NTH_BUTTERWORTH_HIGH
//        || t == T_NTH_BUTTERWORTH_BAND
            );
}


MultiFilter::MultiFilter(bool alloc)
    : doReallocate_ (alloc),
      type_         (T_FIRST_ORDER_LOW),
      sr_           (44100),
      order_        (1),
      freq_         (1000),
      reso_         (0),
      out_amp_      (1),
      cheby_        (doReallocate_? 0 : new ChebychevFilter()),
      filter24_     (doReallocate_? 0 : new Filter24()),
      butter_       (doReallocate_? 0 : new ButterworthFilter()),
      fixed_        (doReallocate_? 0 : new FixedFilter())
{
    reset();
    updateCoefficients();
}

MultiFilter::~MultiFilter()
{
    delete fixed_;
    delete butter_;
    delete filter24_;
    delete cheby_;
}
#if 0
MultiFilter::MultiFilter(const MultiFilter &other)
{
    *this = other;
}

MultiFilter& MultiFilter::operator = (const MultiFilter& other)
{
    type_ = other.type_;
    sr_ = other.sr_;
    freq_ = other.freq_;
    reso_ = other.reso_;
    order_ = other.order_;
    amp_ = other.amp_;

    if (cheby_)
    {
        delete cheby_;
        cheby_ = 0;
    }
    if (filter24_)
    {
        delete filter24_;
        filter24_ = 0;
    }
    if (butter_)
    {
        delete butter_;
        butter_ = 0;
    }
    if (fixed_)
    {
        delete fixed_;
        fixed_ = 0;
    }

    updateCoefficients();

    return *this;
}
#endif

void MultiFilter::copySettingsFrom(const MultiFilter& other)
{
    type_ = other.type_;
    sr_ = other.sr_;
    freq_ = other.freq_;
    reso_ = other.reso_;
    order_ = other.order_;
    amp_ = other.amp_;
}

void MultiFilter::setType(const QString &id)
{
    int idx = filterTypeIds.indexOf(id);
    if (idx >= 0)
        setType((FilterType)filterTypeEnums[idx]);
}

void MultiFilter::reset()
{
    s1_ = s2_ = p0_ = p1_ = 0;
    for (auto & f : so1_)
        f = 0;
    for (auto & f : so2_)
        f = 0;
    for (auto & f : po0_)
        f = 0;
    for (auto & f : po1_)
        f = 0;

    if (cheby_)
        cheby_->reset();
    if (filter24_)
        filter24_->reset();
    if (butter_)
        butter_->reset();
    if (fixed_)
        fixed_->reset();
}

void MultiFilter::updateCoefficients()
{
    MO_ASSERT(sr_ > 0, "samplerate f***ed up");

    // init internal types
    switch (type_)
    {
        case T_BYPASS: break;

        case T_NTH_ORDER_LOW:
        case T_NTH_ORDER_HIGH:
        case T_NTH_ORDER_BAND:
            if (so1_.size() != order_)
            {
                so1_.resize(order_);
                for (auto & f : so1_)
                    f = 0;
            }
            if (so2_.size() != order_)
            {
                so2_.resize(order_);
                for (auto & f : so2_)
                    f = 0;
            }
            if (po0_.size() != order_)
            {
                po0_.resize(order_);
                for (auto & f : po0_)
                    f = 0;
            }
            if (po1_.size() != order_)
            {
                po1_.resize(order_);
                for (auto & f : po1_)
                    f = 0;
            }
        case T_FIRST_ORDER_BAND:
        case T_FIRST_ORDER_LOW:
        case T_FIRST_ORDER_HIGH:
            // frequency coefficient
            q1_ = 1.f - std::min(1.f, std::exp(-2.f * 3.14159265f * freq_ / sr_));
            //q1_ = std::min(1.f, 2.f * freq_ / sr_);
            // resonance coefficient
            q2_ = std::max(0.f, std::min(0.99999f, reso_));
        break;

        case T_24_LOW:
        case T_24_HIGH:
        case T_24_BAND: break;

        case T_CHEBYCHEV_LOW:
        case T_CHEBYCHEV_HIGH:
        case T_CHEBYCHEV_BAND: break;

        case T_BUTTERWORTH_LOW:
        case T_BUTTERWORTH_HIGH: break;

        case T_NTH_BESSEL_LOW:
        case T_NTH_BESSEL_HIGH:
        case T_NTH_BESSEL_BAND:
        case T_NTH_CHEBYCHEV_LOW:
        case T_NTH_CHEBYCHEV_HIGH:
        case T_NTH_CHEBYCHEV_BAND:
        case T_NTH_BUTTERWORTH_LOW:
        case T_NTH_BUTTERWORTH_HIGH:
        case T_NTH_BUTTERWORTH_BAND: break;
    }


    // amplitude adjustment for internal types
    switch (type_)
    {
        case T_FIRST_ORDER_BAND:
        case T_NTH_ORDER_BAND:
            amp_ = F32(1) + q2_ * F32(1./5 - 1);
            amp_ += MATH::smoothstep(F32(.94), F32(1), q2_) * (F32(1./70) - amp_);
        break;

        case T_FIRST_ORDER_LOW:
        case T_NTH_ORDER_LOW:
        {
            F32 fac = std::pow(
                        MATH::smoothstep(F32(0), F32(0.4), q1_) * F32(0.2) + F32(0.8) * q2_
                        , F32(0.5));
            amp_ = F32(1) + fac * F32(1./8 - 1);
            amp_ += MATH::smoothstep(F32(.94), F32(1), q2_) * (F32(1./250) - amp_);
        }
        break;

        case T_FIRST_ORDER_HIGH:
        case T_NTH_ORDER_HIGH:
        {
            F32 fac = std::pow(
                        MATH::smoothstep(F32(0), F32(0.4), q1_) * F32(0.2) + F32(0.8) * q2_
                        , F32(0.5));
            amp_ = F32(1) + fac * F32(1./8 - 1);
            amp_ += MATH::smoothstep(F32(.90), F32(1), q2_) * (F32(1./250) - amp_);
        }

        default:
            amp_ = F32(1);
        break;
    }


    // init chebychev filter
    if (   type_ == T_CHEBYCHEV_LOW
        || type_ == T_CHEBYCHEV_HIGH
        || type_ == T_CHEBYCHEV_BAND)
    {
        if (!cheby_)
            cheby_ = new ChebychevFilter();
        cheby_->setSampleRate(sr_);
        cheby_->setFrequency(freq_);
        cheby_->setResonance(reso_);
        if (type_ == T_CHEBYCHEV_LOW)
            cheby_->setType(ChebychevFilter::T_LOWPASS);
        else if (type_ == T_CHEBYCHEV_HIGH)
            cheby_->setType(ChebychevFilter::T_HIGHPASS);
        if (type_ == T_CHEBYCHEV_BAND)
            cheby_->setType(ChebychevFilter::T_BANDPASS);
        cheby_->updateCoefficients();
    }
    else if (doReallocate_)
    {
        delete cheby_;
        cheby_ = 0;
    }

    // init 24db filter
    if (   type_ == T_24_LOW
        || type_ == T_24_HIGH
        || type_ == T_24_BAND)
    {
        if (!filter24_)
            filter24_ = new Filter24();
        filter24_->setSampleRate(sr_);
        filter24_->setFrequency(freq_);
        filter24_->setResonance(reso_);
        if (type_ == T_24_LOW)
            filter24_->setType(Filter24::T_LOWPASS);
        else if (type_ == T_24_HIGH)
            filter24_->setType(Filter24::T_HIGHPASS);
        if (type_ == T_24_BAND)
            filter24_->setType(Filter24::T_BANDPASS);
        filter24_->updateCoefficients();
    }
    else if (doReallocate_)
    {
        delete filter24_;
        filter24_ = 0;
    }

    // init butterworth filter
    if (   type_ == T_BUTTERWORTH_LOW
        || type_ == T_BUTTERWORTH_HIGH)
    {
        if (!butter_)
            butter_ = new ButterworthFilter();
        butter_->setSampleRate(sr_);
        butter_->setFrequency(freq_);
        butter_->setResonance(reso_);
        if (type_ == T_BUTTERWORTH_LOW)
            butter_->setType(ButterworthFilter::T_LOWPASS);
        else if (type_ == T_BUTTERWORTH_HIGH)
            butter_->setType(ButterworthFilter::T_HIGHPASS);
        butter_->updateCoefficients();
    }
    else if (doReallocate_)
    {
        delete butter_;
        butter_ = 0;
    }

    // init fixed filter
    if ( type_ == T_NTH_BESSEL_LOW
      || type_ == T_NTH_BESSEL_HIGH
      || type_ == T_NTH_BESSEL_BAND
      || type_ == T_NTH_CHEBYCHEV_LOW
      || type_ == T_NTH_CHEBYCHEV_HIGH
      || type_ == T_NTH_CHEBYCHEV_BAND
      || type_ == T_NTH_BUTTERWORTH_LOW
      || type_ == T_NTH_BUTTERWORTH_HIGH
      || type_ == T_NTH_BUTTERWORTH_BAND)
    {
        if (!fixed_)
            fixed_ = new FixedFilter();

        fixed_->setSampleRate(sr_);
        fixed_->setFrequency(freq_);
        fixed_->setOrder(order_);

        switch (type_)
        {
            case T_NTH_BESSEL_LOW:
                fixed_->setType(FixedFilter::FT_BESSEL);
                fixed_->setBandType(FixedFilter::BT_LOWPASS);
            break;
            case T_NTH_BESSEL_HIGH:
                fixed_->setType(FixedFilter::FT_BESSEL);
                fixed_->setBandType(FixedFilter::BT_HIGHPASS);
            break;
            case T_NTH_BESSEL_BAND:
                fixed_->setType(FixedFilter::FT_BESSEL);
                fixed_->setBandType(FixedFilter::BT_BANDPASS);
                /// @todo Use of samplerate here is maybe bad
                fixed_->setBandpassSize(10 + (1.0-reso_) * sr_ / 3);
            break;
            case T_NTH_CHEBYCHEV_LOW:
                fixed_->setType(FixedFilter::FT_CHEBYCHEV);
                fixed_->setBandType(FixedFilter::BT_LOWPASS);
            break;
            case T_NTH_CHEBYCHEV_HIGH:
                fixed_->setType(FixedFilter::FT_CHEBYCHEV);
                fixed_->setBandType(FixedFilter::BT_HIGHPASS);
            break;
            case T_NTH_CHEBYCHEV_BAND:
                fixed_->setType(FixedFilter::FT_CHEBYCHEV);
                fixed_->setBandType(FixedFilter::BT_BANDPASS);
                fixed_->setBandpassSize(10 + (1.0-reso_) * sr_ / 3);
            break;
            case T_NTH_BUTTERWORTH_LOW:
                fixed_->setType(FixedFilter::FT_BUTTERWORTH);
                fixed_->setBandType(FixedFilter::BT_LOWPASS);
            break;
            case T_NTH_BUTTERWORTH_HIGH:
                fixed_->setType(FixedFilter::FT_BUTTERWORTH);
                fixed_->setBandType(FixedFilter::BT_HIGHPASS);
            break;
            case T_NTH_BUTTERWORTH_BAND:
                fixed_->setType(FixedFilter::FT_BUTTERWORTH);
                fixed_->setBandType(FixedFilter::BT_BANDPASS);
                fixed_->setBandpassSize(10 + (1.0-reso_) * sr_ / 3);
            break;
            default: break;
        }

        fixed_->updateCoefficients();
    }
    else if (doReallocate_)
    {
        delete fixed_;
        fixed_ = 0;
    }
}

void MultiFilter::process(const F32 *input, uint inputStride,
                                F32 *output, uint outputStride, uint blockSize)
{
    F32 amp = amp_ * out_amp_;

    switch (type_)
    {
        case T_BYPASS:
            for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                *output = *input;
        break;

        case T_FIRST_ORDER_LOW:
            // without resonance
            if (!q2_)
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    *output = s1_ += q1_ * (*input - s1_);
                }
            // with resonance
            else
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    p0_ = p1_;
                    p1_ = s1_;
                    *output = (s1_ += q1_ * (*input - s1_)
                                    + q2_ * (s1_ - p0_)) * amp;
                }
        break;

        case T_FIRST_ORDER_HIGH:
            if (!q2_)
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    s1_ += q1_ * (*input - s1_);
                    // highpass == input minus lowpass
                    *output = *input - s1_;
                }
            else
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    p0_ = p1_;
                    p1_ = s1_;
                    s1_ += q1_ * (*input - s1_)
                         + q2_ * (s1_ - p0_);
                    // highpass == input minus lowpass
                    *output = (*input - s1_) * amp;
                }
        break;

        case T_FIRST_ORDER_BAND:
            if (!q2_)
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    s1_ += q1_ * (*input - s1_);
                    // lowpass of highpass
                    s2_ += q1_ * ((*input - s1_) - s2_);
                    *output = s2_;
                }
            else
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    s1_ += q1_ * (*input - s1_);
                    // lowpass of highpass
                    p0_ = p1_;
                    p1_ = s2_;
                    s2_ += q1_ * ((*input - s1_) - s2_)
                         + q2_ * (s2_ - p0_);
                    *output = s2_ * amp;
                }
        break;


        case T_NTH_ORDER_LOW:
            if (!q2_)
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    so1_[0] += q1_ * (*input - so1_[0]);

                    for (uint j=1; j<so1_.size(); ++j)
                    {
                        so1_[j] += q1_ * (so1_[j-1] - so1_[j]);
                    }
                    *output = so1_[order_-1];
                }
            else
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    po0_[0] = po1_[0];
                    po1_[0] = so1_[0];
                    so1_[0] += q1_ * (*input - so1_[0])
                             + q2_ * (so1_[0] - po0_[0]);

                    for (uint j=1; j<so1_.size(); ++j)
                    {
                        po0_[j] = po1_[j];
                        po1_[j] = so1_[j];
                        so1_[j] += (q1_ * (so1_[j-1] - so1_[j])
                                  + q2_ * (so1_[j] - po0_[j])) * amp_;
                    }
                    *output = so1_[order_-1] * amp;
                }
        break;

        case T_NTH_ORDER_HIGH:
            if (!q2_)
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    so1_[0] += q1_ * (*input - so1_[0]);
                    // highpass == input minus lowpass
                    F32 tmp = *input - so1_[0];

                    for (uint j=1; j<so1_.size(); ++j)
                    {
                        so1_[j] += q1_ * (tmp - so1_[j]);
                        tmp = tmp - so1_[j];
                    }
                    *output = tmp * amp;
                }
            else
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    po0_[0] = po1_[0];
                    po1_[0] = so1_[0];
                    so1_[0] += q1_ * (*input - so1_[0])
                             + q2_ * (so1_[0] - po0_[0]);
                    // highpass == input minus lowpass
                    F32 tmp = *input - so1_[0];

                    for (uint j=1; j<so1_.size(); ++j)
                    {
                        po0_[j] = po1_[j];
                        po1_[j] = so1_[j];
                        so1_[j] += q1_ * (tmp - so1_[j])
                                 + q2_ * (so1_[j] - po0_[j]);
                        tmp = (tmp - so1_[j]) * amp_;
                    }
                    *output = tmp * amp;
                }
        break;

        case T_NTH_ORDER_BAND:
            if (!q2_)
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    so1_[0] += q1_ * (*input - so1_[0]);
                    // lowpass of highpass
                    so2_[0] += q1_ * ((*input - so1_[0]) - so2_[0]);

                    for (uint j=1; j<so1_.size(); ++j)
                    {
                        so1_[j] += q1_ * (so2_[j-1] - so1_[j]);
                        so2_[j] += q1_ * ((so2_[j-1] - so1_[j]) - so2_[j]);
                    }
                    *output = so2_[order_-1] * amp;
                }
            else
                for (uint i=0; i<blockSize; ++i, input += inputStride, output += outputStride)
                {
                    so1_[0] += q1_ * (*input - so1_[0]);
                    // lowpass of highpass (with resonance)
                    po0_[0] = po1_[0];
                    po1_[0] = so2_[0];
                    so2_[0] += q1_ * ((*input - so1_[0]) - so2_[0])
                             + q2_ * (so2_[0] - po0_[0]);

                    for (uint j=1; j<so1_.size(); ++j)
                    {
                        so1_[j] += q1_ * (so2_[j-1] - so1_[j]);
                        po0_[j] = po1_[j];
                        po1_[j] = so2_[j];
                        so2_[j] += (q1_ * ((so2_[j-1] - so1_[j]) - so2_[j])
                                  + q2_ * (so2_[j] - po0_[j])) * amp_;
                    }
                    *output = so2_[order_-1] * amp;
                }
        break;

        case T_CHEBYCHEV_LOW:
        case T_CHEBYCHEV_HIGH:
        case T_CHEBYCHEV_BAND:
            MO_ASSERT(cheby_, "forgot to call MultiFilter::updateCoefficients() ?");
            cheby_->process(input, inputStride, output, outputStride, blockSize, out_amp_);
        break;

        case T_24_LOW:
        case T_24_HIGH:
        case T_24_BAND:
            MO_ASSERT(filter24_, "forgot to call MultiFilter::updateCoefficients() ?");
            filter24_->process(input, inputStride, output, outputStride, blockSize, out_amp_);
        break;

        case T_BUTTERWORTH_LOW:
        case T_BUTTERWORTH_HIGH:
            MO_ASSERT(butter_, "forgot to call MultiFilter::updateCoefficients() ?");
            butter_->process(input, inputStride, output, outputStride, blockSize, out_amp_);
        break;

        case T_NTH_BESSEL_LOW:
        case T_NTH_BESSEL_HIGH:
        case T_NTH_BESSEL_BAND:
        case T_NTH_CHEBYCHEV_LOW:
        case T_NTH_CHEBYCHEV_HIGH:
        case T_NTH_CHEBYCHEV_BAND:
        case T_NTH_BUTTERWORTH_LOW:
        case T_NTH_BUTTERWORTH_HIGH:
        case T_NTH_BUTTERWORTH_BAND:
            MO_ASSERT(fixed_, "forgot to call MultiFilter::updateCoefficients() ?");
            fixed_->process(input, inputStride, output, outputStride, blockSize, out_amp_);
        break;

    }
}

} // namespace AUDIO
} // namespace MO
