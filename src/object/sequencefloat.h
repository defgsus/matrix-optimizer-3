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
#include "audio/tool/waveform.h"
#include "audio/audio_fwd.h"

namespace PPP_NAMESPACE { class Parser; }
namespace MO {
namespace MATH { class Timeline1D; }

class SequenceFloat : public Sequence
{
    Q_OBJECT
public:

    enum SequenceType
    {
        ST_CONSTANT,
        ST_TIMELINE,
        ST_OSCILLATOR,
        ST_OSCILLATOR_WT,
        ST_SPECTRAL_OSC,
        ST_SPECTRAL_WT,
        ST_SOUNDFILE,
        ST_EQUATION,
        ST_EQUATION_WT
    };
    const static int ST_MAX = ST_EQUATION_WT + 1;

    /** PERSISTENT ids of the sequence types */
    static QStringList sequenceTypeId;
    /** friendly names of the sequence types */
    static QStringList sequenceTypeName;

    static bool typeUsesFrequency(SequenceType t)
    {
        return     t == ST_OSCILLATOR
                || t == ST_OSCILLATOR_WT
                || t == ST_SPECTRAL_OSC
                || t == ST_SPECTRAL_WT
                || t == ST_EQUATION_WT;
    }

    bool typeUsesFrequency() const { return typeUsesFrequency((SequenceType)p_mode_->baseValue()); }

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
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;


    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;

    // ------------ getter --------------

    /** The sequence mode - one of the SequenceType enums */
    SequenceType sequenceType() const
        { return (SequenceType)p_mode_->baseValue(); }

    AUDIO::Waveform::Type oscillatorMode() const
    { return (AUDIO::Waveform::Type)p_oscMode_->baseValue(); }

    /** Returns the constant offset added to the output */
    Double offset() const { return p_offset_->baseValue(); }

    /** Returns the amplitude, applied before the constant offset */
    Double amplitude() const { return p_amplitude_->baseValue(); }

    /** Returns the frequency of the oscillator in Hertz. */
    Double frequency() const { return p_frequency_->baseValue(); }
    /** Returns the phase of the oscillator [0,1] */
    Double phase() const { return p_phase_->baseValue(); }
    /** Returns the pulsewidth of the oscillator [0,1] */
    Double pulseWidth() const { return p_pulseWidth_->baseValue(); }

    /** Returns either 1 or 1/360, depending on the phaseInDegree mode. */
    Double phaseMultiplier() const { return phaseMult_; }

    Double specNumPartials() const { return p_specNum_->baseValue(); }
    Double specOctaveStep() const { return p_specOct_->baseValue(); }
    Double specAmplitudeMultiplier() const { return p_specAmp_->baseValue(); }
    Double specPhase() const { return p_specPhase_->baseValue(); }
    Double specPhaseShift() const { return p_specPhaseShift_->baseValue(); }

    /** Wheter the loop start/end are overlapping. */
    LoopOverlapMode loopOverlapMode() const
        { return (LoopOverlapMode)p_loopOverlapMode_->baseValue(); }

    /** Overlapping time of loop in seconds */
    Double loopOverlap() const { return p_loopOverlap_->baseValue(); }

    /** A value that is added to the blended value in the transition window */
    Double loopOverlapOffset() const { return p_loopOverlapOffset_->baseValue(); }

    const QString & equationText() const;
    bool useFrequency() const { return p_useFreq_->baseValue(); }
    bool phaseInDegree() const { return p_doPhaseDegree_->baseValue(); }

    /** Returns the minimum and maximum values across the time range (local) */
    void getMinMaxValue(Double localStart, Double localEnd,
                        Double& minValue, Double& maxValue, uint thread) const;

    /** Returns access to the wavetable generator, or NULL if not initialized */
    AUDIO::WavetableGenerator * wavetableGenerator() const { return wavetableGen_; }

    /** Returns access to the wavetable, or NULL if not initialized */
    AUDIO::Wavetable<Double> * wavetable() const { return wavetable_; }

    // ------------ setter --------------

    void setMode(SequenceType m) { p_mode_->setValue(m); updateValueObjects_(); }

    void setOscillatorMode(AUDIO::Waveform::Type mode) { p_oscMode_->setValue(mode); }

    void setOffset(Double o) { p_offset_->setValue(o); }
    void setAmplitude(Double a) { p_amplitude_->setValue(a); }

    void setFrequency(Double f) { p_frequency_->setValue(f); }
    void setPhase(Double p) { p_phase_->setValue(p); }
    void setPulseWidth(Double pw) { p_pulseWidth_->setValue(AUDIO::Waveform::limitPulseWidth(pw)); }

    void setSpecNumPartials(Double num) { p_specNum_->setValue(num); }
    void setSpecOctaveStep(Double step) { p_specOct_->setValue(step); }
    void setSpecAmplitudeMultiplier(Double mul) { p_specAmp_->setValue(mul); }
    void setSpecPhase(Double p) { p_specPhase_->setValue(p); }
    void setSpecPhaseShift(Double p) { p_specPhaseShift_->setValue(p); }

    void setLoopOverlap(Double t)
        { p_loopOverlap_->setValue( std::max(minimumLength(), t) ); }
    void setLoopOverlapOffset(Double v)
        { p_loopOverlapOffset_->setValue( v ); }

    void setLoopOverlapMode(LoopOverlapMode mode)
        { p_loopOverlapMode_->setValue(mode); }

    void setEquationText(const QString&);

    void setUseFrequency(bool enable) { p_useFreq_->setValue(enable); }

    // ------------ values --------------

    MATH::Timeline1D * timeline() { return timeline_; }
    const MATH::Timeline1D * timeline() const { return timeline_; }
    PPP_NAMESPACE::Parser * equation(uint thread);
    const PPP_NAMESPACE::Parser * equation(uint thread) const;

    Double value(Double time, uint thread) const;

signals:

public slots:

private:

    void updateValueObjects_();
    /** Updates the internal wavetable from the WavetableGenerator settings.
        @note mode() must be ST_SPECTRAL_WT */
    void updateWavetable_();
    void updatePhaseInDegree_();

    Double value_(Double gtime, Double time, uint thread) const;

    MATH::Timeline1D * timeline_;
    AUDIO::Wavetable<Double> * wavetable_;
    AUDIO::WavetableGenerator * wavetableGen_;
    AUDIO::BandlimitWavetableGenerator * waveformGen_;
    AUDIO::SoundFile * soundFile_;

    class SeqEquation;
    std::vector<SeqEquation *> equation_;

    ParameterFilename * p_soundFile_;

    ParameterFloat
        * p_offset_,
        * p_amplitude_,

        * p_frequency_,
        * p_phase_,
        * p_pulseWidth_,

        * p_specNum_,
        * p_specOct_,
        * p_specPhase_,
        * p_specPhaseShift_,
        * p_specAmp_,

        * p_wtSpecPhase_,
        * p_wtSpecPhaseShift_,
        * p_wtSpecAmp_,

        * p_loopOverlap_,
        * p_loopOverlapOffset_;

    ParameterInt
        * p_wtSpecNum_,
        * p_wtSpecOct_,
        * p_wtSpecOctStep_;

    ParameterSelect
        * p_wtSize_,
        * p_oscWtSize_,
        * p_wtSpecSize_,
        * p_mode_,
        * p_oscMode_,
        * p_loopOverlapMode_,
        * p_useFreq_,
        * p_doPhaseDegree_;

    ParameterText
        * p_equationText_,
        * p_wtEquationText_;

    Double phaseMult_;

};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCEFLOAT_H
