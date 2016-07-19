/** @file geometrymodifiercreate.h

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERCREATE_H
#define MOSRC_GEOM_GEOMETRYMODIFIERCREATE_H

#include "GeometryModifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierCreate : public GeometryModifier
{
public:

    /** Order doesn't matter for persistence
        as long as typeIds is maintained */
    enum Type
    {
        T_FILE_OBJ,
#ifndef MO_DISABLE_SHP
        T_FILE_SHP,
#endif
        T_QUAD,
        T_TETRAHEDRON,
        T_BOX,
        T_BOX_UV,
        T_OCTAHEDRON,
        T_ICOSAHEDRON,
        T_DODECAHEDRON,
        T_CYLINDER,
        T_CONE,
        T_TORUS,
        T_UV_SPHERE,
        T_GRID_XZ,
        T_POINT_GRID,
        T_LINE_GRID,
        T_QUAD_GRID
    };
    static Properties::NamedValues namedTypes();
    /** XXX For backward compatibility - since Properties */
    static const QStringList typeIds;

    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierCreate)

    // ------------- getter ------------------

    bool isFile() const;
    QString filename() const;

    // ------------ setter -------------------

private:

};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERCREATE_H
