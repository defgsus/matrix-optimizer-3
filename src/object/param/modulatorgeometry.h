/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATORGEOMETRY_H
#define MOSRC_OBJECT_PARAM_MODULATORGEOMETRY_H

#include "modulator.h"
#include "gl/opengl_fwd.h"
#include "types/time.h"

namespace MO {

class ValueGeometryInterface;

/** A connection to pass GEOM::Geometry pointers.
*/
class ModulatorGeometry : public Modulator
{
public:

    /** Construct a modulator coming from object @p modulatorId
        and belonging to @p parent. */
    ModulatorGeometry(const QString& name, const QString& modulatorId, const QString &outputId,
                   Parameter * p, Object * parent = 0);

    // --------------- io ----------------

    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

    // ------------- getter --------------

    /** Returns if the object can be the modulating object */
    virtual bool canBeModulator(const Object *) const Q_DECL_OVERRIDE;

    /** Returns the modulation-value at given time,
        or NULL if there is no texture. */
    const GEOM::Geometry* value(const RenderTime & time) const;

    // ------------- setter ----------------

    virtual void copySettingsFrom(const Modulator * other) Q_DECL_OVERRIDE;

protected:

    virtual void modulatorChanged_() Q_DECL_OVERRIDE;

private:

    ValueGeometryInterface * geomFace_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_MODULATORGEOMETRY_H
