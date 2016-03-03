/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERPYTHON34_H
#define MOSRC_GEOM_GEOMETRYMODIFIERPYTHON34_H

#include "geometrymodifier.h"

namespace MO {
namespace GUI { class Python34Widget; }
namespace GEOM {


class GeometryModifierPython34 : public GeometryModifier
{
public:

    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierPython34)

    // ------------- getter ------------------

    // ------------ setter -------------------

private:
    GUI::Python34Widget* widget_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERPYTHON34_H

#endif // MO_ENABLE_PYTHON34
