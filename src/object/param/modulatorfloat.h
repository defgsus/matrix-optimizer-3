/** @file modulatorfloat.h

    @brief Float modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATORFLOAT_H
#define MOSRC_OBJECT_PARAM_MODULATORFLOAT_H

#include "modulator.h"
#include "types/float.h"

namespace MO {

class ModulatorFloat : public Modulator
{
public:

    /** Type of source */
    enum SourceType
    {
        ST_NONE,
        ST_SEQUENCE_FLOAT,
        ST_TRACK_FLOAT,
        ST_MODULATOR_OBJECT_FLOAT
        // for conversion XX not implemented yet
        //,ST_AUDIO_OBJECT
    };

    /** Construct a modulator coming form object @p modulatorId
        and belonging to @p parent */
    ModulatorFloat(const QString& name, const QString& modulatorId, const QString &outputId,
                   Parameter * p, Object * parent = 0);

    // --------------- io ----------------

    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

    // ------------- getter --------------



    /** Returns if the object can be the modulating object */
    virtual bool canBeModulator(const Object *) const Q_DECL_OVERRIDE;

    /** Returns the type of the assigned modulator object.
        Valid after Modulator::setModulator() */
    SourceType sourceType() const { return sourceType_; }

    /** Returns true if the SourceType supports amplitude modulations */
    bool hasAmplitude() const;
    /** Returns true if the SourceType supports timeoffset modulations */
    bool hasTimeOffset() const;

    /** Returns the modulation-value at given time */
    Double value(Double time, uint thread) const;

    /** Returns the amplitude of the modulation value */
    Double amplitude() const { return amplitude_; }

    /** Returns the time offset to apply when reading from tracks or sequences. */
    Double timeOffset() const { return timeOffset_; }

    // ------------- setter ----------------

    virtual void copySettingsFrom(const Modulator * other) Q_DECL_OVERRIDE;

    void setAmplitude(Double amp) { amplitude_ = amp; }
    void setTimeOffset(Double off) { timeOffset_ = off; }

protected:

    virtual void modulatorChanged_() Q_DECL_OVERRIDE;

private:

    SourceType sourceType_;

    Double amplitude_, timeOffset_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_MODULATORFLOAT_H
