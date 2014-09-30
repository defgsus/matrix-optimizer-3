/** @file butterworthfilter.h

    @brief Linkwitz-Riley filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.09.2014</p>

    adopted from:
        http://musicdsp.org/archive.php?classid=3#266
*/

#ifndef MOSRC_AUDIO_TOOL_BUTTERWORTHFILTER_H
#define MOSRC_AUDIO_TOOL_BUTTERWORTHFILTER_H


#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {


/** 4th order Linkwitz-Riley filter.

    Currently very over-exciting for frequencies < 200 and > nyquist - 200
    */
class ButterworthFilter
{
public:

    enum FilterType
    {
        T_LOWPASS,
        T_HIGHPASS,
        T_BANDPASS
    };

    ButterworthFilter();

    // ----------- setter ----------------

    /** Sets the samplerate in hertz.
        Requires updateCoefficients() to be called. */
    void setSampleRate(uint sr) { sr_ = sr; }

    /** Sets the frequency in hertz.
        Requires updateCoefficients() to be called. */
    void setFrequency(F32 freq) { freq_ = freq; }

    /** Sets the resonance [0,1].
        Requires updateCoefficients() to be called. */
    void setResonance(F32 reso) { reso_ = reso; }

    /** Sets the minimum/maximum value for input/output samples */
    void setClipping(F32 clip) { clip_ = clip; }

    /** Sets highpass or lowpass mode. */
    void setType(FilterType type) { type_ = type; }

    // ---------- getter ------------------

    uint sampleRate() const { return sr_; }
    F32 frequency() const { return freq_; }
    F32 resonance() const { return reso_; }
    F32 clipping() const { return clip_; }
    FilterType type() const { return type_; }

    // ---------- processing --------------

    /** Resets the filter temporaries.
        In case a NAN has crawled in. */
    void reset();

    /** Call this after changes to the filter settings */
    void updateCoefficients();

    /** Consequently call this to filter the input signal */
    void process(const F32 * input, F32 * output, uint blockSize)
        { process(input, 1, output, 1, blockSize); }

    /** Consequently call this to filter the input signal.
        The strides define the spacing between consecutive samples. */
    void process(const F32 * input, uint inputStride,
                       F32 * output, uint outputStride,
                       uint blockSize);

private:

    FilterType type_;

    uint sr_;
    F32 freq_, reso_, clip_,
        q_, amp_,
        a0_, a1_, a2_, a3_, a4_,
        ha0_, ha1_, ha2_, ha3_, ha4_,
        b1_, b2_, b3_, b4_,
        xm1_, xm2_, xm3_, xm4_,
        ym1_, ym2_, ym3_, ym4_,
        hxm1_, hxm2_, hxm3_, hxm4_,
        hym1_, hym2_, hym3_, hym4_;

};

} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_BUTTERWORTHFILTER_H
