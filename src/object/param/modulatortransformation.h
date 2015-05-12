/** @file modulatortransformation.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATORTRANSFORMATION_H
#define MOSRC_OBJECT_PARAM_MODULATORTRANSFORMATION_H

#include "modulator.h"
#include "types/vector.h"

namespace MO {

class ValueTransformationInterface;

class ModulatorTransformation : public Modulator
{
public:

    /** Construct a modulator coming from object @p modulatorId
        and belonging to @p parent.
        @p outputId should be an int for each audio channel in case case of conversion from AudioObject. */
    ModulatorTransformation(
            const QString &name, const QString &modulatorId, const QString &outputId, Parameter *p, Object *parent = 0);

    // --------------- io ----------------

    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

    // ------------- getter --------------

    /** Returns if the object can be the modulating object */
    virtual bool canBeModulator(const Object *) const Q_DECL_OVERRIDE;

    uint channel() const { return channel_; }

    /** Returns true if the SourceType supports amplitude modulations */
    bool hasAmplitude() const { return false; }
    /** Returns true if the SourceType supports timeoffset modulations */
    bool hasTimeOffset() const { return false; }

    /** Returns the modulation-value at given time */
    Mat4 value(Double time, uint thread) const;

    // ------------- setter ----------------

    virtual void copySettingsFrom(const Modulator * other) Q_DECL_OVERRIDE;

protected:

    virtual void modulatorChanged_() Q_DECL_OVERRIDE;

private:

    uint channel_;
    ValueTransformationInterface * interface_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_MODULATORTRANSFORMATION_H
