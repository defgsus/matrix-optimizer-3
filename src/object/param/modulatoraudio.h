/** @file modulatoraudio.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATORAUDIO_H
#define MOSRC_OBJECT_PARAM_MODULATORAUDIO_H

#include "modulator.h"
#include "types/float.h"

namespace MO {

class ModulatorAudio : public Modulator
{
public:

    /** Type of source */
    enum SourceType
    {
        ST_NONE,
        ST_SEQUENCE_FLOAT,
        ST_TRACK_FLOAT,
        ST_MODULATOR_OBJECT_FLOAT
    };

    /** Construct a modulator coming form object @p modulatorId
        and belonging to @p parent */
    ModulatorAudio(const QString& name, const QString& modulatorId, Object * parent = 0);

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

    /** Returns number of channels */
    uint numChannels() const;

    uint bufferSize() const;



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

#endif // MOSRC_OBJECT_PARAM_MODULATORAUDIO_H
