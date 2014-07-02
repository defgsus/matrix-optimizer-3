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

    explicit SequenceFloat(QObject *parent = 0);

    MO_OBJECT_CLONE(SequenceFloat)

    const QString& className() const { static QString s(MO_OBJECTCLASSNAME_SEQUENCE_FLOAT); return s; }

    virtual Type type() const { return T_SEQUENCE_FLOAT; }

    // --------- io ---------------------

    virtual void serialize(IO::DataStream &) const;
    virtual void deserialize(IO::DataStream &);

    // ------------ getter --------------

    /** The sequence mode - one of the SequenceType enums */
    SequenceType mode() const { return mode_; }

    MATH::Waveform::Type oscillatorMode() const { return oscMode_; }

    /** Returns the constant offset added to the output */
    Double offset() const { return offset_; }

    /** Returns the amplitude, applied before the constant offset */
    Double amplitude() const { return amplitude_; }

    // ------------ setter --------------

    void setMode(SequenceType);

    void setOscillatorMode(MATH::Waveform::Type);

    void setOffset(Double o) { offset_ = o; }

    void setAmplitude(Double a) { amplitude_ = a; }

    // ------------ values --------------

    MATH::Timeline1D * timeline() { return timeline_; }
    const MATH::Timeline1D * timeline() const { return timeline_; }

    Double value(Double time) const;

signals:

public slots:

private:

    SequenceType mode_;
    MATH::Timeline1D * timeline_;

    double offset_,
           amplitude_,

           frequency_,
           phase_,
           pulseWidth_;

    MATH::Waveform::Type oscMode_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SEQUENCEFLOAT_H
