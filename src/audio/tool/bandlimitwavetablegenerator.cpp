/** @file bandlimitwavetablegenerator.cpp

    @brief Generator for wavetables with bandlimited common waveforms

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.10.2014</p>
*/

/* Note:
 *
 * http://musicdsp.org/showArchiveComment.php?ArchiveID=134
 * states that he made good experience with 32x oversampling and
 * a 511-tap FIR (blackman-harris) 12 khz cutoff.
 * maybe try that once...
 *
 */

#include "bandlimitwavetablegenerator.h"
#include "fixedfilter.h"
#include "math/constants.h"

namespace MO {
namespace AUDIO {

BandlimitWavetableGenerator::BandlimitWavetableGenerator()
    : tableSize_        (nextPowerOfTwo(32000)),
      oversampling_     (4),
      waveType_         (Waveform::T_SQUARE),
      pulseWidth_       (0.5),
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

void BandlimitWavetableGenerator::setPulseWidth(Double pw)
{
    pulseWidth_ = Waveform::limitPulseWidth(pw);
    needWaveCalc_ = true;
}


void BandlimitWavetableGenerator::recalc_()
{
    if (!needRecalc_)
        return;

    // suppose tableSize_ * oversampling_ is exactly one second/period
    const uint hz = tableSize_ * oversampling_;

    table_.resize(hz);
    ftable_.resize(hz);

    // setup filter
    filter_->setType(FixedFilter::FT_BUTTERWORTH);
    filter_->setBandType(FixedFilter::BT_LOWPASS);
    filter_->setOrder(6);
    // some experimental values for samplerate and frequency
    // (suppose that the entire wavetable is a waveform at 1 hz)
    filter_->setSampleRate(tableSize_ * 2);
    filter_->setFrequency(tableSize_ / 60);
    filter_->updateCoefficients();

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
        table_[i] = Waveform::waveform(Double(i) / hz, waveType_, pulseWidth_);
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
