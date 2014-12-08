/** @file fixedfilter.h

    @brief Butterworth/Chebychev/Bessel filter with coefficient generator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.09.2014</p>

    <p>adapted from: Dr Anthony J. Fisher / http://www-users.cs.york.ac.uk/~fisher/software/mkfilter</p>
*/

#ifndef MOSRC_AUDIO_TOOL_FIXEDFILTER_H
#define MOSRC_AUDIO_TOOL_FIXEDFILTER_H


#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {


/** Very accurate Butterworth/Chebychev/Bessel filters.

    Computation of coefficients is very demanding.
    Not so much a filter for audio manipulation but rather for audio analyzation.

    */
class FixedFilter
{
public:

    enum BandType
    {
        BT_LOWPASS,
        BT_HIGHPASS,
        BT_BANDPASS
    };

    enum FilterType
    {
        FT_BESSEL,
        FT_CHEBYCHEV,
        FT_BUTTERWORTH
    };

    FixedFilter();
    ~FixedFilter();

    // ----------- setter ----------------

    /** Sets the samplerate in Hertz.
        Requires updateCoefficients() to be called. */
    void setSampleRate(uint sr);

    /** Sets the order of the filter.
        Requires updateCoefficients() to be called. */
    void setOrder(uint order);

    /** Sets the frequency in Hertz.
        Requires updateCoefficients() to be called. */
    void setFrequency(Double freq);

    /** Sets the size of the passband in Hertz.
        Requires updateCoefficients() to be called. */
    void setBandpassSize(Double freq);

    /** Sets the type of filter.
        Requires updateCoefficients() to be called. */
    void setType(FilterType type);

    /** Sets the type of filter band.
        Requires updateCoefficients() to be called. */
    void setBandType(BandType type);

    /** Sets the ripple of chebychev filters in Decibel.
        Requires updateCoefficients() to be called. */
    void setChebychevRipple(Double db);

    /** Sets the minimum/maximum value for input/output samples */
    void setClipping(Double clip);

    // ---------- getter ------------------

    uint sampleRate() const;
    uint order() const;
    Double frequency() const;
    Double bandpassSize() const;
    FilterType type() const;
    BandType bandType() const;
    Double chebychevRipple() const;
    Double clipping() const;

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

    /** Consequently call this to filter the input signal */
    void process(const Double * input, Double * output, uint blockSize, Double amp = Double(1))
        { process(input, 1, output, 1, blockSize, amp); }

    /** Consequently call this to filter the input signal.
        The strides define the spacing between consecutive samples. */
    void process(const Double * input, uint inputStride,
                       Double * output, uint outputStride,
                       uint blockSize, Double amp = Double(1));

private:

    class Private;
    Private * p_;
};

} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_FIXEDFILTER_H
