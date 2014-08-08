/** @file multifilter.h

    @brief Multi-mode audio filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_MULTIFILTER_H
#define MOSRC_AUDIO_TOOL_MULTIFILTER_H

#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {


class MultiFilter
{
public:
    MultiFilter();

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

    // ---------- getter ------------------

    uint sampleRate() const { return sr_; }

    F32 frequency() const { return freq_; }
    F32 resonance() const { return reso_; }

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

    uint sr_;
    F32 freq_, reso_,
        q1_,
        s1_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_MULTIFILTER_H
