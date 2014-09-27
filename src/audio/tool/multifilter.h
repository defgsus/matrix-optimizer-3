/** @file multifilter.h

    @brief Multi-mode audio filter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_MULTIFILTER_H
#define MOSRC_AUDIO_TOOL_MULTIFILTER_H

#include <QStringList>

#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {

class ChebychevFilter;
class Filter24;
class ButterworthFilter;
class FixedFilter;

class MultiFilter
{
public:

    enum FilterType
    {
        T_BYPASS,
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
        T_BUTTERWORTH_BAND,
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

    static const QStringList filterTypeIds;
    static const QStringList filterTypeNames;
    static const QStringList filterTypeStatusTips;
    static const QList<int>  filterTypeEnums;

    /** Returns true when the given filter-type is an 'nth order type' */
    static bool supportsOrder(FilterType);

    /** Returns true when the given filter-type has a resonance setting */
    static bool supportsResonance(FilterType);

    /** If @p dynamicAllocation is false, individual sub-classes
        like the ChebychevFilter will be created in the constructor,
        if true, they will be created and deleted as needed by
        updateCoefficients() */
    MultiFilter(bool dynamicAllocation = true);

    ~MultiFilter();

    MultiFilter(const MultiFilter& other);
    MultiFilter& operator = (const MultiFilter& other);

    // ----------- setter ----------------

    /** Sets the type of the filter.
        Requires updateCoefficients() to be called. */
    void setType(FilterType type) { type_ = type; }

    /** Sets the type of the filter, if id is known.
        Requires updateCoefficients() to be called. */
    void setType(const QString& id);

    /** Sets the order of the T_NTH_ORDER type filters.
        Requires updateCoefficients() to be called. */
    void setOrder(uint order) { order_ = std::max((uint)1, order); }

    /** Sets the samplerate in hertz.
        Requires updateCoefficients() to be called. */
    void setSampleRate(uint sr) { sr_ = sr; }

    /** Sets the frequency in hertz.
        Requires updateCoefficients() to be called. */
    void setFrequency(F32 freq) { freq_ = freq; }

    /** Sets the resonance [0,1].
        Requires updateCoefficients() to be called. */
    void setResonance(F32 reso) { reso_ = std::max(0.f,std::min(1.f, reso )); }

    // ---------- getter ------------------

    FilterType type() const { return type_; }
    const QString& typeName() const { return filterTypeNames[type_]; }
    const QString& typeId() const { return filterTypeIds[type_]; }

    uint order() const { return order_; }

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

    bool doReallocate_;

    FilterType type_;

    uint sr_, order_;
    F32 freq_, reso_,
        q1_, q2_, s1_, s2_, p0_, p1_;
    std::vector<F32> so1_, so2_, po0_, po1_;

    ChebychevFilter * cheby_;
    Filter24 * filter24_;
    ButterworthFilter * butter_;
    FixedFilter * fixed_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_MULTIFILTER_H
