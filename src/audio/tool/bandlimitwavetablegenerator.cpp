/** @file bandlimitwavetablegenerator.cpp

    @brief Generator for wavetables with bandlimited common waveforms

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.10.2014</p>
*/

#include "bandlimitwavetablegenerator.h"
#include "fixedfilter.h"
#include "math/constants.h"

namespace MO {
namespace AUDIO {

BandlimitWavetableGenerator::BandlimitWavetableGenerator()
    : tableSize_        (nextPowerOfTwo(32000)),
      oversampling_     (3),
      waveType_         (Waveform::T_SQUARE),
      needRecalc_       (true),
      needWaveCalc_     (true),
      filter_           (new FixedFilter())
{
}

BandlimitWavetableGenerator::~BandlimitWavetableGenerator()
{
    delete filter_;
}


void BandlimitWavetableGenerator::setTableSize(uint size)
{
    tableSize_ = nextPowerOfTwo(size);
    needRecalc_ = true;
}

void BandlimitWavetableGenerator::setOversampling(uint over)
{
    oversampling_ = std::max(2u, over);
    needRecalc_ = true;
}

void BandlimitWavetableGenerator::setWaveform(Waveform::Type type)
{
    waveType_ = type;
    needWaveCalc_ = true;
}


void BandlimitWavetableGenerator::recalc_()
{
    if (!needRecalc_)
        return;

    // suppose tableSize_ * oversampling_ is exactly one second
    const uint hz = tableSize_ * oversampling_;

    table_.resize(hz);
    ftable_.resize(hz);

#if (0)
    filter_->setBandType(FixedFilter::BT_BANDPASS);
    filter_->setSampleRate(hz);
    filter_->setOrder(6);
    // set the allowed band to the nyquist range
    filter_->setFrequency(hz/4);
    filter_->setBandpassSize(hz/4.1);
    filter_->updateCoefficients();
#else
    filter_->setType(FixedFilter::FT_BUTTERWORTH);
    filter_->setBandType(FixedFilter::BT_LOWPASS);
    filter_->setOrder(6);
    // some experimental values for samplerate and frequency
    // (suppose that the entire wavetable is a waveform at 1 hz)
    filter_->setSampleRate(tableSize_ * 2);
    filter_->setFrequency(tableSize_ / 60);
    filter_->updateCoefficients();
#endif

    needRecalc_ = false;
    needWaveCalc_ = true;
}

void BandlimitWavetableGenerator::generateTable_()
{
    if (!needWaveCalc_)
        return;

    const uint hz = tableSize_ * oversampling_;

    // generate it
    for (uint i=0; i<hz; ++i)
    {
        table_[i] = Waveform::waveform(Double(i) / hz, waveType_);
    }

    // filter it

    filter_->reset();
    // one pass to fill filter history
    filter_->process(&table_[0], &ftable_[0], hz);
    // one real pass
    filter_->process(&table_[0], &ftable_[0], hz);

    needWaveCalc_ = false;
}



} // namespace AUDIO
} // namespace MO
