/** @file modulatorevent.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.12.2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATOREVENT_H
#define MOSRC_OBJECT_PARAM_MODULATOREVENT_H

#include "modulator.h"
#include "types/float.h"
#include "types/int.h"

namespace MO {

// XXX NOT USED YET

class ModulatorEvent : public Modulator
{
public:

    /** Type of source */
    enum Type
    {
        T_NONE,
        T_FLOAT
    };

    struct Event
    {
        Double time;
        union
        {
            Double vfloat;
            Int vint;
        };
    };

    /** Construct a modulator connecting object @p modulatorId
        with Parameter @p p belonging to object @p parent */
    ModulatorEvent(const QString& name, const QString& modulatorId, const QString &outputId,
                   Parameter * p, Object * parent = 0);

    // --------------- io ----------------

    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

    // ------------- getter --------------

    /** Returns if the object can be the modulating object */
    virtual bool canBeModulator(const Object *) const Q_DECL_OVERRIDE;

    /** Returns the type of data
        Valid after Modulator::setModulator() */
    Type type() const { return type_; }

    bool isValid() const { return type_ != T_NONE; }
    bool isFloat() const { return type_ == T_FLOAT; }

    /** Returns true if the Type supports amplitude modulations */
    bool hasAmplitude() const;
    bool hasTimeOffset() const;

    /** Returns the amplitude of the modulation value */
    Double amplitude() const { return amplitude_; }
    Double timeOffset() const { return timeOffset_; }

    // ------------- setter ----------------

    virtual void copySettingsFrom(const Modulator * other) Q_DECL_OVERRIDE;

    void setAmplitude(Double amp) { amplitude_ = amp; }
    void setTimeOffset(Double off) { timeOffset_ = off; }

    // -------------------- event ---------------------------

    /** Return the closest event equal to or before time */
    const Event * getEvent(Double time) const;

    /** push value into consideration.
        Must be called in increasing time order to make reading work. */
    void setValue(Double time, Double value);

    /** Returns the modulation-value since given time */
    Double valueFloat(Double time, uint thread) const;

protected:

    virtual void modulatorChanged_() Q_DECL_OVERRIDE;

private:

    Type type_;

    Double amplitude_, timeOffset_;

    std::vector<Event> buffer_;
    size_t bufPos_, mask_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_MODULATOREVENT_H
