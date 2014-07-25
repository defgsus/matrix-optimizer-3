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
        ST_WAVETABLE_GEN,
        ST_EQUATION
    };
    const static int ST_MAX = ST_EQUATION + 1;

    /** PERSITANT ids of the sequence types */
    static QStringList sequenceTypeId;
    /** friendly names of the sequence types */
    static QStringList sequenceTypeName;

    enum SequenceTypeGroups
    {
        /** All types that use frequency/phase */
        STG_FREQUENCY = ST_OSCILLATOR | ST_WAVETABLE_GEN
    };


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

    virtual void createParameters();

    // ------------ getter --------------

    /** The sequence mode - one of the SequenceType enums */
    SequenceType mode() const { return mode_; }

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
                        Double& minValue, Double& maxValue) const;

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
        @note mode() MUST be ST_WAVETABLE_GEN */
    void updateWavetable();

    // ------------ values --------------

    MATH::Timeline1D * timeline() { return timeline_; }
    const MATH::Timeline1D * timeline() const { return timeline_; }
    PPP_NAMESPACE::Parser * equation() { return equation_; }
    const PPP_NAMESPACE::Parser * equation() const { return equation_; }

    Double value(Double time) const;

signals:

public slots:

private:

    Double value_(Double gtime, Double time) const;

    SequenceType mode_;
    MATH::Timeline1D * timeline_;
    AUDIO::Wavetable<Double> * wavetable_;
    AUDIO::WavetableGenerator * wavetableGen_;
    PPP_NAMESPACE::Parser * equation_;


    ParameterFloat
        * offset_,
        * amplitude_,

        * frequency_,
        * phase_,
        * pulseWidth_,

        * loopOverlap_,
        * loopOverlapOffset_;

    AUDIO::Waveform::Type oscMode_;
    LoopOverlapMode loopOverlapMode_;

    bool doUseFreq_,
         doPhaseDegree_;

    Double phaseMult_;

    // ----- equation stuff -----

    QString equationText_;
    mutable Double
        equationTime_,
        equationFreq_,
        equationPhase_,
        equationPW_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCEFLOAT_H
