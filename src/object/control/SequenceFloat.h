/** @file sequencefloat.h

    @brief Float sequence

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_SEQUENCEFLOAT_H
#define MOSRC_OBJECT_SEQUENCEFLOAT_H

#include <mutex>

#include <QStringList>

#include "Sequence.h"
#include "object/interface/ValueFloatInterface.h"
#include "audio/tool/Waveform.h"
#include "audio/audio_fwd.h"

namespace PPP_NAMESPACE { class Parser; }
namespace MO {
namespace MATH { class Timeline1d; }

class SequenceFloat : public Sequence, public ValueFloatInterface
{
public:

    enum SequenceType
    {
        ST_CONSTANT,
        ST_TIMELINE,
        ST_OSCILLATOR,
        ST_OSCILLATOR_WT,
        ST_ADD_OSC,
        ST_ADD_WT,
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
                || t == ST_ADD_OSC
                || t == ST_ADD_WT
                || t == ST_SPECTRAL_WT
                || t == ST_EQUATION_WT;
    }

    bool typeUsesFrequency() const
        { return typeUsesFrequency((SequenceType)p_mode_->baseValue()); }

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

    virtual Type type() const { return T_SEQUENCE_FLOAT; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void copyFrom(const Object *other) Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;

    virtual SequenceFloat* splitSequence(Double localTime) Q_DECL_OVERRIDE;

    // ------------ getter --------------

    /** The sequence mode - one of the SequenceType enums */
    SequenceType sequenceType() const
        { return (SequenceType)p_mode_->baseValue(); }

    AUDIO::SoundFile* soundFile() const { return soundFile_; }

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

#if 0
    /** Returns access to the wavetable generator, or NULL if not initialized */
    AUDIO::WavetableGenerator * wavetableGenerator() const { return wavetableGen_; }

    /** Returns access to the wavetable, or NULL if not initialized */
    AUDIO::Wavetable<Double> * wavetable() const { return wavetable_; }
#endif
    // ------------ setter --------------

    void setSequenceType(SequenceType m);

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
    void setSoundFilename(const QString&);

    void setUseFrequency(bool enable) { p_useFreq_->setValue(enable); }

    // ------------ values --------------

    /** Copies the timeline data and sets the sequenceType to ST_TIMELINE if necessary */
    void setTimeline(const MATH::Timeline1d&);
    /** Adds the timeline data to the internal timeline and
        sets the sequenceType to ST_TIMELINE if necessary.
        If @p adjustLength is true, the sequence start and end will be adjusted
        to contain the timeline if necessary. */
    void addTimeline(const MATH::Timeline1d&, Double timeOffset, bool adjustLength = true);
    /** Adds the timeline to the internal timeline overwrites deletes any previous
        data that might have been there in the range of the given timeline.
        Sets the sequenceType to ST_TIMELINE if necessary.
        If @p adjustLength is true, the sequence start and end will be adjusted
        to contain the timeline if necessary. */
    void overwriteTimeline(const MATH::Timeline1d&, Double timeOffset, bool adjustLength = true);

    MATH::Timeline1d * timeline() { return timeline_; }
    const MATH::Timeline1d * timeline() const { return timeline_; }
    PPP_NAMESPACE::Parser * equation(uint thread);
    const PPP_NAMESPACE::Parser * equation(uint thread) const;

    // ------- ValueFloatInterface --------

    Double valueFloat(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;
    /** Returns the minimum and maximum values across the time range (local) */
    void getValueFloatRange(
                uint channel, const RenderTime& time, Double length,
                Double* minimum, Double* maximum) const override;

signals:

public slots:

private:

    void updateValueObjects_();
    /** Updates the internal wavetable from the WavetableGenerator settings.
        @note mode() must be ST_SPECTRAL_WT */
    void updateWavetable_();
    void updatePhaseInDegree_();

    Double value_(const RenderTime& time) const;
    Double fade_(const RenderTime& time) const;

    MATH::Timeline1d * timeline_;
    AUDIO::Wavetable<Double> * wavetable_;
    AUDIO::SoundFile * soundFile_;

    class SeqEquation;
    std::vector<SeqEquation*> equation_;

    ParameterFilename * p_soundFile_;

    // one cache value for all threads
    Double cacheValue_, cacheTime_;

    Double phaseMult_;

    mutable std::mutex cacheMutex_;
    mutable std::vector<Double>
        lastMinMaxStart_, lastMinMaxLength_,
        lastMinValue_, lastMaxValue_;

    ParameterFloat
        * p_offset_,
        * p_amplitude_,

        * p_frequency_,
        * p_phase_,
        * p_pulseWidth_,
        * p_smooth_,
        * p_oscWtPulseWidth_,

        * p_specNum_,
        * p_specOct_,
        * p_specPhase_,
        * p_specPhaseShift_,
        * p_specAmp_,

        * p_wtSpecPhase_,
        * p_wtSpecPhaseShift_,
        * p_wtSpecAmp_,

        * p_loopOverlap_,
        * p_loopOverlapOffset_,

        * p_fadeIn_,
        * p_fadeOut_;

    ParameterInt
        * p_wtSpecNum_,
        * p_wtSpecOct_,
        * p_wtSpecOctStep_,
        * p_soundFileChannel_;

    ParameterSelect
        * p_wtSize_,
        * p_oscWtSize_,
        * p_wtSpecSize_,
        * p_mode_,
        * p_oscMode_,
        * p_loopOverlapMode_,
        * p_useFreq_,
        * p_doPhaseDegree_,
        * p_fadeMode_;

    ParameterText
        * p_equationText_,
        * p_wtEquationText_;

    ParameterTimeline1D
        * p_wtFreqs_,
        * p_wtPhases_;


};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCEFLOAT_H
