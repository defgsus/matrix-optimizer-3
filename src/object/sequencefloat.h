/** @file sequencefloat.h

    @brief Float sequence

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_SEQUENCEFLOAT_H
#define MOSRC_OBJECT_SEQUENCEFLOAT_H

#include <QStringList>

#include "sequence.h"
#include "audio/waveform.h"

namespace PPP_NAMESPACE { class Parser; }
namespace MO {
namespace MATH { class Timeline1D; }
namespace AUDIO { template <typename F> class Wavetable; class WavetableGenerator; }

class SequenceFloat : public Sequence
{
    Q_OBJECT
public:

    enum SequenceType
    {
        ST_CONSTANT,
        ST_TIMELINE,
        ST_OSCILLATOR,
        ST_SPECTRAL_OSC,
        ST_SPECTRAL_WT,
        ST_EQUATION
    };
    const static int ST_MAX = ST_EQUATION + 1;

    /** PERSISTENT ids of the sequence types */
    static QStringList sequenceTypeId;
    /** friendly names of the sequence types */
    static QStringList sequenceTypeName;

    static bool typeUsesFrequency(SequenceType t)
    {
        return t == ST_OSCILLATOR || t == ST_SPECTRAL_OSC
                || t == ST_SPECTRAL_WT;
    }

    bool typeUsesFrequency() const { return typeUsesFrequency(mode_); }

    enum LoopOverlapMode
    {
        LOT_OFF,
        LOT_BEGIN,
        LOT_END
    };
    const static int LOT_MAX = LOT_END + 1;

    /** PERSITANT ids of the sequence types */
    static QStringList loopOverlapModeId;
    /** friendly names of the sequence types */
    static QStringList loopOverlapModeName;



    // -------------- ctor --------------

    MO_OBJECT_CONSTRUCTOR(SequenceFloat);
    ~SequenceFloat();

    virtual Type type() const { return T_SEQUENCE_FLOAT; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    // ------------ getter --------------

    /** The sequence mode - one of the SequenceType enums */
    SequenceType sequenceType() const { return mode_; }

    AUDIO::Waveform::Type oscillatorMode() const { return oscMode_; }

    /** Returns the constant offset added to the output */
    Double offset() const { return offset_->baseValue(); }

    /** Returns the amplitude, applied before the constant offset */
    Double amplitude() const { return amplitude_->baseValue(); }

    /** Returns the frequency of the oscillator in Hertz. */
    Double frequency() const { return frequency_->baseValue(); }
    /** Returns the phase of the oscillator [0,1] */
    Double phase() const { return phase_->baseValue(); }
    /** Returns the pulsewidth of the oscillator [0,1] */
    Double pulseWidth() const { return pulseWidth_->baseValue(); }

    /** Returns either 1 or 1/360, depending on the phaseInDegree mode. */
    Double phaseMultiplier() const { return phaseMult_; }

    Double specNumPartials() const { return specNum_->baseValue(); }
    Double specOctaveStep() const { return specOct_->baseValue(); }
    Double specAmplitudeMultiplier() const { return specAmp_->baseValue(); }
    Double specPhase() const { return specPhase_->baseValue(); }
    Double specPhaseShift() const { return specPhaseShift_->baseValue(); }

    /** Wheter the loop start/end are overlapping. */
    LoopOverlapMode loopOverlapMode() const { return loopOverlapMode_; }

    /** Overlapping time of loop in seconds */
    Double loopOverlap() const { return loopOverlap_->baseValue(); }

    /** A value that is added to the blended value in the transition window */
    Double loopOverlapOffset() const { return loopOverlapOffset_->baseValue(); }

    const QString& equationText() const { return equationText_; }
    bool useFrequency() const { return doUseFreq_; }
    bool phaseInDegree() const { return doPhaseDegree_; }

    /** Returns the minimum and maximum values across the time range (local) */
    void getMinMaxValue(Double localStart, Double localEnd,
                        Double& minValue, Double& maxValue, uint thread) const;

    /** Returns access to the wavetable generator, or NULL if not initialized */
    AUDIO::WavetableGenerator * wavetableGenerator() const { return wavetableGen_; }

    /** Returns access to the wavetable, or NULL if not initialized */
    AUDIO::Wavetable<Double> * wavetable() const { return wavetable_; }

    // ------------ setter --------------

    void setMode(SequenceType);

    void setOscillatorMode(AUDIO::Waveform::Type mode) { oscMode_ = mode; }

    void setOffset(Double o) { offset_->setValue(o); }
    void setAmplitude(Double a) { amplitude_->setValue(a); }

    void setFrequency(Double f) { frequency_->setValue(f); }
    void setPhase(Double p) { phase_->setValue(p); }
    void setPulseWidth(Double pw) { pulseWidth_->setValue(AUDIO::Waveform::limitPulseWidth(pw)); }

    void setSpecNumPartials(Double num) { specNum_->setValue(num); }
    void setSpecOctaveStep(Double step) { specOct_->setValue(step); }
    void setSpecAmplitudeMultiplier(Double mul) { specAmp_->setValue(mul); }
    void setSpecPhase(Double p) { specPhase_->setValue(p); }
    void setSpecPhaseShift(Double p) { specPhaseShift_->setValue(p); }

    void setLoopOverlap(Double t)
        { loopOverlap_->setValue( std::max(minimumLength(), t) ); }
    void setLoopOverlapOffset(Double v)
        { loopOverlapOffset_->setValue( v ); }

    void setLoopOverlapMode(LoopOverlapMode mode)
        { loopOverlapMode_ = mode; }

    void setEquationText(const QString&);

    void setUseFrequency(bool enable) { doUseFreq_ = enable; }
    void setPhaseInDegree(bool enable);

    /** Updates the internal wavetable from the WavetableGenerator settings.
        @note mode() MUST be ST_SPECTRAL_WT */
    void updateWavetable();

    // ------------ values --------------

    MATH::Timeline1D * timeline() { return timeline_; }
    const MATH::Timeline1D * timeline() const { return timeline_; }
    PPP_NAMESPACE::Parser * equation(uint thread);
    const PPP_NAMESPACE::Parser * equation(uint thread) const;

    Double value(Double time, uint thread) const;

signals:

public slots:

private:

    Double value_(Double gtime, Double time, uint thread) const;

    SequenceType mode_;
    MATH::Timeline1D * timeline_;
    AUDIO::Wavetable<Double> * wavetable_;
    AUDIO::WavetableGenerator * wavetableGen_;

    class SeqEquation;
    std::vector<SeqEquation *> equation_;

    ParameterFloat
        * offset_,
        * amplitude_,

        * frequency_,
        * phase_,
        * pulseWidth_,

        * specNum_,
        * specOct_,
        * specPhase_,
        * specPhaseShift_,
        * specAmp_,

        * loopOverlap_,
        * loopOverlapOffset_;

    AUDIO::Waveform::Type oscMode_;
    LoopOverlapMode loopOverlapMode_;

    bool doUseFreq_,
         doPhaseDegree_;

    Double phaseMult_;

    // ----- equation stuff -----

    QString equationText_;

};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCEFLOAT_H
