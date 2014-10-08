/** @file bandlimitwavetablegenerator.h

    @brief Generator for wavetables with bandlimited common waveforms

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.10.2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_BANDLIMITWAVETABLEGENERATOR_H
#define MOSRC_AUDIO_TOOL_BANDLIMITWAVETABLEGENERATOR_H

#include <vector>

#include "wavetable.h"
#include "waveform.h"
#include "types/float.h"
#include "types/int.h"

namespace MO {
namespace AUDIO {

class FixedFilter;

/** Generator for wavetables with bandlimited common waveforms */
class BandlimitWavetableGenerator
{
public:
    BandlimitWavetableGenerator();
    ~BandlimitWavetableGenerator();

    // ------------- getter ----------------

    uint tableSize() const { return tableSize_; }
    uint oversampling() const { return oversampling_; }

    // ------------- setter ----------------

    /** Sets the size of the tables.
        Will be rounded to the next power of two */
    void setTableSize(uint size);

    /** Sets the amount of oversampling */
    void setOversampling(uint over);

    /** Sets the type of waveform to render */
    void setWaveform(Waveform::Type);

    // ------------ wavetables -------------

    /** Generates the wavetable with the given waveform of size tableSize() */
    template <typename F>
    void createWavetable(Wavetable<F> & wt);

private:

    void recalc_();
    void generateTable_();

    uint tableSize_, oversampling_;
    std::vector<Double> table_, ftable_;

    Waveform::Type waveType_;

    bool needRecalc_,
         needWaveCalc_;
    FixedFilter * filter_;
};



// ___________________________ template IMPL ___________________________

template <typename F>
void BandlimitWavetableGenerator::createWavetable(Wavetable<F> &wt)
{
    recalc_();
    generateTable_();

    // downsample into wavetable

    wt.setSize(tableSize_);

    Double * in = &ftable_[0];
    F      * out = wt.data();

    for (uint i=0; i<tableSize_; ++i, ++out)
    {
        Double sum = 0.0;
        for (uint j=0; j<oversampling_; ++j, ++in)
            sum += *in;

        *out = sum / oversampling_;
    }
}




} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_BANDLIMITWAVETABLEGENERATOR_H
