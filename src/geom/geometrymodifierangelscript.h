/** @file geometrymodifierangelscript.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERANGELSCRIPT_H
#define MOSRC_GEOM_GEOMETRYMODIFIERANGELSCRIPT_H

#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierAngelScript : public GeometryModifier
{
public:

    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierAngelScript)

    // ------------- getter ------------------

    const QString& script() const { return script_; }

    // ------------ setter -------------------

    void setScript(const QString& script) { script_ = script; }

private:

    QString script_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERANGELSCRIPT_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
