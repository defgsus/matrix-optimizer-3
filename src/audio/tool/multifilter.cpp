/** @file multifilter.cpp

    @brief Multi-mode audio filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "multifilter.h"
#include "chebychevfilter.h"
#include "io/error.h"

namespace MO {
namespace AUDIO {


const QStringList MultiFilter::filterTypeIds =
{ "bypass", "low", "high", "band",
  "nlow", "nhigh", "nband",
  "cheblow", "chebhigh", "chebband" };

const QStringList MultiFilter::filterTypeNames =
{ QObject::tr("off"),
  QObject::tr("1st order low-pass"),
  QObject::tr("1st order high-pass"),
  QObject::tr("1st order band-pass"),
  QObject::tr("nth order low-pass"),
  QObject::tr("nth order high-pass"),
  QObject::tr("nth order band-pass"),
  QObject::tr("2nd chebychev low"),
  QObject::tr("2nd chebychev high"),
  QObject::tr("2nd chebychev band") };

const QStringList MultiFilter::filterTypeStatusTips =
{ QObject::tr("No filtering takes place"),
  QObject::tr("1st order low-pass"),
  QObject::tr("1st order high-pass"),
  QObject::tr("1st order band-pass"),
  QObject::tr("nth order low-pass"),
  QObject::tr("nth order high-pass"),
  QObject::tr("nth order band-pass"),
  QObject::tr("2nd order, 24db/oct, chebychev low-pass"),
  QObject::tr("2nd order, 24db/oct, chebychev high-pass"),
  QObject::tr("2nd order, 24db/oct, chebychev band-pass") };

const QList<int> MultiFilter::filterTypeEnums =
{ T_BYPASS,
  T_FIRST_ORDER_LOW,
  T_FIRST_ORDER_HIGH,
  T_FIRST_ORDER_BAND,
  T_NTH_ORDER_LOW,
  T_NTH_ORDER_HIGH,
  T_NTH_ORDER_BAND,
  T_CHEBYCHEV_LOW,
  T_CHEBYCHEV_HIGH,
  T_CHEBYCHEV_BAND };

bool MultiFilter::supportsOrder(FilterType t)
{
    return t == T_NTH_ORDER_LOW
        || t == T_NTH_ORDER_HIGH
        || t == T_NTH_ORDER_BAND;
}

MultiFilter::MultiFilter(bool alloc)
    : doReallocate_ (alloc),
      type_         (T_FIRST_ORDER_LOW),
      sr_           (44100),
      order_        (1),
      freq_         (1000),
      reso_         (0),
      cheby_        (doReallocate_? 0 : new ChebychevFilter())
{
    reset();
    updateCoefficients();
}

MultiFilter::~MultiFilter()
{
    delete cheby_;
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
        case T_FIRST_ORDER_LOW:
        case T_FIRST_ORDER_HIGH:
        case T_FIRST_ORDER_BAND:
            q1_ = std::min(1.f, 2.f * freq_ / sr_);
            q2_ = std::max(0.f, std::min(0.99999f, reso_));
        break;

        case T_CHEBYCHEV_LOW:
        case T_CHEBYCHEV_HIGH:
        case T_CHEBYCHEV_BAND: break;
    }

    // init chebychev filters
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
}

void MultiFilter::process(const F32 *input, uint inputStride,
                                F32 *output, uint outputStride, uint blockSize)
{
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
                    *output = s1_ += q1_ * (*input - s1_)
                                   + q2_ * (s1_ - p0_);
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
                    *output = *input - s1_;
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
                    *output = s2_;
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
                        so1_[j] += q1_ * (so1_[j-1] - so1_[j])
                                 + q2_ * (so1_[j] - po0_[j]);
                    }
                    *output = so1_[order_-1];
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
                    *output = tmp;
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
                        tmp = tmp - so1_[j];
                    }
                    *output = tmp;
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
                    *output = so2_[order_-1];
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
                        so2_[j] += q1_ * ((so2_[j-1] - so1_[j]) - so2_[j])
                                 + q2_ * (so2_[j] - po0_[j]);
                    }
                    *output = so2_[order_-1];
                }
        break;

        case T_CHEBYCHEV_LOW:
        case T_CHEBYCHEV_HIGH:
        case T_CHEBYCHEV_BAND:
            MO_ASSERT(cheby_, "forgot to call MultiFilter::updateCoefficients() ?");
            cheby_->process(input, inputStride, output, outputStride, blockSize);
        break;

    }
}

} // namespace AUDIO
} // namespace MO
