/** @file geometryfactory.h

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYFACTORY_H
#define MOSRC_GEOM_GEOMETRYFACTORY_H

#include <QStringList>

#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace IO { class DataStream; }
namespace GEOM {

class Geometry;
class GeometryFactorySettings;
class ObjLoader;

class GeometryFactory
{
public:

    // --------------------- static functions ------------------

    static void createFromSettings(Geometry *, const GeometryFactorySettings *, ObjLoader *);

    static void createQuad(Geometry *, float side_length_x, float side_length_y,
                           bool asTriangles = true);

    static void createCube(Geometry *, float side_length, bool asTriangles = true);
    static void createBox(Geometry *,
            float side_length_x, float side_length_y, float side_length_z,
                          bool asTriangles = true);

    static void createUVSphere(Geometry *, float rad, uint segu, uint segv,
                               bool asTriangles = true);
    static void createUVSphereLines(Geometry *, float rad, uint segu, uint segv);

    static void createTetrahedron(Geometry *, float scale, bool asTriangles = true);
    static void createOctahedron(Geometry *, float scale, bool asTriangles = true);
    static void createIcosahedron(Geometry *, float scale, bool asTriangles = true);
    static void createDodecahedron(Geometry *, float scale, bool asTriangles = true);

    static void createGridXZ(Geometry *, int sizeX, int sizeZ, bool with_coordinate_system);
    static void createGrid(Geometry *, int sizeX, int sizeY, int sizeZ);

};



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
        T_GRID_XZ,
        T_GRID,
        T_UV_SPHERE
    };
    static const uint numTypes = T_UV_SPHERE + 1;
    static const QStringList typeIds;
    static const QStringList typeNames;

    GeometryFactorySettings();

    void serialize(IO::DataStream&) const;
    void deserialize(IO::DataStream&);

    // ---- public member ----

    Type type;
    QString filename;
    bool calcNormals, asTriangles, convertToLines, sharedVertices, tesselate,
        normalizeVertices, removeRandomly;
    Float colorR, colorG, colorB, colorA;
    Float scale, scaleX, scaleY, scaleZ, removeProb, normalization;
    uint gridSize, segmentsX, segmentsY, segmentsZ, tessLevel, removeSeed;

    bool withCoords;

};


} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_GEOMETRYFACTORY_H
