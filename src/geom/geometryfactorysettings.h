/** @file geometryfactorysettings.h

    @brief Settings for dynamically creating Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYFACTORYSETTINGS_H
#define MOSRC_GEOM_GEOMETRYFACTORYSETTINGS_H

#include <QStringList>

#include "types/int.h"
#include "types/float.h"

#include "geometrymodifierchain.h"

namespace MO {
namespace IO { class DataStream; }
namespace GEOM {

class GeometryFactorySettings
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

    GeometryFactorySettings();
    ~GeometryFactorySettings();

    // ---------- io ---------

    void serialize(IO::DataStream&) const;
    void deserialize(IO::DataStream&);

    void saveFile(const QString& filename) const;
    void loadFile(const QString& filename);


    // ---- public member ----

    GeometryModifierChain modifierChain;

    Type type;
    QString filename, equationX, equationY, equationZ,
                     pEquationX, pEquationY, pEquationZ;
    bool calcNormals, invertNormals, asTriangles, convertToLines, sharedVertices, tesselate,
        normalizeVertices, removeRandomly, transformWithEquation,
        transformPrimitivesWithEquation, calcNormalsBeforePrimitiveEquation,
        extrude;
    Float colorR, colorG, colorB, colorA;
    Float scale, scaleX, scaleY, scaleZ, removeProb, normalization, smallRadius,
        extrudeConstant, extrudeFactor;
    uint gridSize, segmentsX, segmentsY, segmentsZ, tessLevel, removeSeed;

    bool withCoords;

};


} // namespace GEOM
} // namespace MO

#endif // GEOMETRYFACTORYSETTINGS_H
