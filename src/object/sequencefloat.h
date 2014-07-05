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
#include "math/waveform.h"

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
        ST_EQUATION
    };
    const static int ST_MAX = ST_EQUATION + 1;

    /** PERSITANT ids of the sequence types */
    static QStringList sequenceTypeId;
    /** friendly names of the sequence types */
    static QStringList sequenceTypeName;

    // -------------- ctor --------------

    MO_OBJECT_CONSTRUCTOR(SequenceFloat);
    ~SequenceFloat();

    virtual Type type() const { return T_SEQUENCE_FLOAT; }

    // ------------ getter --------------

    /** The sequence mode - one of the SequenceType enums */
    SequenceType mode() const { return mode_; }

    MATH::Waveform::Type oscillatorMode() const { return oscMode_; }

    /** Returns the constant offset added to the output */
    Double offset() const { return offset_; }

    /** Returns the amplitude, applied before the constant offset */
    Double amplitude() const { return amplitude_; }

    /** Returns the frequency of the oscillator in Hertz. */
    Double frequency() const { return frequency_; }
    /** Returns the phase of the oscillator [0,1] */
    Double phase() const { return phase_; }
    /** Returns the pulsewidth of the oscillator [0,1] */
    Double pulseWidth() const { return pulseWidth_; }

    const QString& equationText() const { return equationText_; }
    bool equationWithFreq() const { return doEquationFreq_; }

    // ------------ setter --------------

    void setMode(SequenceType);

    void setOscillatorMode(MATH::Waveform::Type mode) { oscMode_ = mode; }

    void setOffset(Double o) { offset_ = o; }
    void setAmplitude(Double a) { amplitude_ = a; }

    void setFrequency(Double f) { frequency_ = f; }
    void setPhase(Double p) { phase_ = p; }
    void setPulseWidth(Double pw) { pulseWidth_ = MATH::Waveform::limitPulseWidth(pw); }

    void setEquationText(const QString&);
    void setEquationWithFreq(bool enable) { doEquationFreq_ = enable; }

    // ------------ values --------------

    MATH::Timeline1D * timeline() { return timeline_; }
    const MATH::Timeline1D * timeline() const { return timeline_; }
    PPP_NAMESPACE::Parser * equation() { return equation_; }
    const PPP_NAMESPACE::Parser * equation() const { return equation_; }

    Double value(Double time) const;

signals:

public slots:

private:

    SequenceType mode_;
    MATH::Timeline1D * timeline_;
    PPP_NAMESPACE::Parser * equation_;

    double offset_,
           amplitude_,

           frequency_,
           phase_,
           pulseWidth_;

    MATH::Waveform::Type oscMode_;

    // ----- equation stuff -----

    bool doEquationFreq_;
    QString equationText_;
    mutable Double
        equationTime_,
        equationFreq_,
        equationPhase_,
        equationPW_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCEFLOAT_H
