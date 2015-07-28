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
        T_CYLINDER_CLOSED,
        T_CYLINDER_OPEN,
        T_CONE_CLOSED,
        T_CONE_OPEN,
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

    Type type() const { return type_; }
    const QString& filename() const { return filename_; }
    bool asTriangles() const { return asTriangles_; }
    bool sharedVertices() const { return sharedVertices_; }
    Float red() const { return colorR_; }
    Float green() const { return colorG_; }
    Float blue() const { return colorB_; }
    Float alpha() const { return colorA_; }
    uint segmentsX() const { return segmentsX_; }
    uint segmentsY() const { return segmentsY_; }
    uint segmentsZ() const { return segmentsZ_; }
    Float smallRadius() const { return smallRadius_; }

    // ------------ setter -------------------

    void setType(Type t) { type_ = t; }
    void setAsTriangles(bool v) { asTriangles_ = v; }
    void setSharedVertices(bool v) { sharedVertices_ = v; }
    void setFilename(const QString& filename) { filename_ = filename; }
    void setRed(Float v) { colorR_ = v; }
    void setGreen(Float v) { colorG_ = v; }
    void setBlue(Float v) { colorB_ = v; }
    void setAlpha(Float v) { colorA_ = v; }
    void setSegmentsX(uint x) { segmentsX_ = x; }
    void setSegmentsY(uint x) { segmentsY_ = x; }
    void setSegmentsZ(uint x) { segmentsZ_ = x; }
    void setSmallRadius(Float r) { smallRadius_ = r; }

private:

    QString filename_;
    Type type_;
    bool asTriangles_, sharedVertices_;
    Float colorR_, colorG_, colorB_, colorA_;
    uint segmentsX_, segmentsY_, segmentsZ_;
    Float smallRadius_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERCREATE_H
