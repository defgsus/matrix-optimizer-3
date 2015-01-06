/** @file chebychevfilter.h

    @brief 2nd order 24db/oct chebychev filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/12/2014</p>

    adapted from:
        http://musicdsp.org/archive.php?classid=3#229
        http://musicdsp.org/showArchiveComment.php?ArchiveID=229
*/

#ifndef MOSRC_AUDIO_TOOL_CHEBYCHEVFILTER_H
#define MOSRC_AUDIO_TOOL_CHEBYCHEVFILTER_H

#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {


class ChebychevFilter
{
public:

    enum FilterType
    {
        T_LOWPASS,
        T_HIGHPASS,
        T_BANDPASS
    };

    ChebychevFilter();

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
    void process(const F32 * input, F32 * output, uint blockSize, F32 amp = F32(1))
        { process(input, 1, output, 1, blockSize, amp); }

    /** Consequently call this to filter the input signal.
        The strides define the spacing between consecutive samples. */
    void process(const F32 * input, uint inputStride,
                       F32 * output, uint outputStride,
                       uint blockSize, F32 amp = F32(1));

private:

    FilterType type_;

    uint sr_;
    F32 freq_, reso_, clip_,
        a0_, a1_, a2_, a3_, a4_, a5_,
        b0_, b1_, b2_, b3_, b4_, b5_,
        stage0_, stage1_, state0_, state1_, state2_, state3_,
        bstage0_, bstage1_, bstate0_, bstate1_, bstate2_, bstate3_;

};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_CHEBYCHEVFILTER_H
