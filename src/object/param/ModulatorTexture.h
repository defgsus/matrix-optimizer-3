/** @file modulatortexture.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.05.2015</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATORTEXTURE_H
#define MOSRC_OBJECT_PARAM_MODULATORTEXTURE_H

#include "Modulator.h"
#include "types/time.h"
#include "gl/opengl_fwd.h"

namespace MO {

class ValueTextureInterface;

/** A connection to pass pointers to GL::Texture.

    @todo better orthogonize Output/Modulator structure in general.
    .. before it get's to painful
*/
class ModulatorTexture : public Modulator
{
public:

    /** Construct a modulator coming from object @p modulatorId
        and belonging to @p parent. */
    ModulatorTexture(const QString& name, const QString& modulatorId, const QString &outputId,
                   Parameter * p, Object * parent = 0);

    // --------------- io ----------------

    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

    // ------------- getter --------------


    /** Returns if the object can be the modulating object */
    virtual bool canBeModulator(const Object *) const Q_DECL_OVERRIDE;

    /** Returns the modulation-value at given time,
        or NULL if there is no texture. */
    const GL::Texture* value(const RenderTime & time) const;

    // ------------- setter ----------------

    virtual void copySettingsFrom(const Modulator * other) Q_DECL_OVERRIDE;

protected:

    virtual void modulatorChanged_() Q_DECL_OVERRIDE;

private:

    ValueTextureInterface * texFace_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_MODULATORTEXTURE_H
