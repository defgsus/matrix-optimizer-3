/** @file geometrymodifiercreate.h

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERCREATE_H
#define MOSRC_GEOM_GEOMETRYMODIFIERCREATE_H

#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierCreate : public GeometryModifier
{
public:

    enum Type
    {
        T_FILE,
        T_QUAD,
        T_TETRAHEDRON,
        T_BOX,
        T_OCTAHEDRON,
        T_ICOSAHEDRON,
        T_DODECAHEDRON,
        T_CYLINDER_CLOSED,
        T_CYLINDER_OPEN,
        T_TORUS,
        T_UV_SPHERE,
        T_GRID_XZ,
        T_LINE_GRID
    };
    static const uint numTypes = T_LINE_GRID + 1;
    static const QStringList typeIds;
    static const QStringList typeNames;

    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierCreate)

    // ------------- getter ------------------

    const QString& getFilename() const { return filename_; }

    // ------------ setter -------------------

    void setFilename(const QString& filename) { filename_ = filename; }

private:

    QString filename_;
    Type type_;

    bool asTriangles_, sharedVertices_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERCREATE_H
