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

    static void createQuad(Geometry *, Float side_length_x, Float side_length_y,
                           bool asTriangles = true);

    static void createCube(Geometry *, Float side_length, bool asTriangles = true);
    static void createBox(Geometry *,
            Float side_length_x, Float side_length_y, Float side_length_z,
                          bool asTriangles = true);

    static void createUVSphere(Geometry *, Float rad, uint segu, uint segv,
                               bool asTriangles = true);
    static void createUVSphereLines(Geometry *, Float rad, uint segu, uint segv);

    static void createCylinder(Geometry *, Float rad, Float height, uint segu, uint segv, bool open,
                               bool asTriangles);

    static void createTorus(Geometry *, Float rad_outer, Float rad_inner, uint segu, uint segv,
                            bool asTriangles);


    static void createTetrahedron(Geometry *, Float scale, bool asTriangles = true);
    static void createOctahedron(Geometry *, Float scale, bool asTriangles = true);
    static void createIcosahedron(Geometry *, Float scale, bool asTriangles = true);
    static void createDodecahedron(Geometry *, Float scale, bool asTriangles = true);

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
        T_CYLINDER_CLOSED,
        T_CYLINDER_OPEN,
        T_TORUS,
        T_GRID_XZ,
        T_GRID,
        T_UV_SPHERE
    };
    static const uint numTypes = T_UV_SPHERE + 1;
    static const QStringList typeIds;
    static const QStringList typeNames;

    GeometryFactorySettings();

    // ---------- io ---------

    void serialize(IO::DataStream&) const;
    void deserialize(IO::DataStream&);

    void saveFile(const QString& filename) const;
    void loadFile(const QString& filename);


    // ---- public member ----

    Type type;
    QString filename, equationX, equationY, equationZ,
                     pEquationX, pEquationY, pEquationZ;
    bool calcNormals, asTriangles, convertToLines, sharedVertices, tesselate,
        normalizeVertices, removeRandomly, transformWithEquation,
                    transformPrimitivesWithEquation, calcNormalsBeforePrimitiveEquation;
    Float colorR, colorG, colorB, colorA;
    Float scale, scaleX, scaleY, scaleZ, removeProb, normalization, smallRadius;
    uint gridSize, segmentsX, segmentsY, segmentsZ, tessLevel, removeSeed;

    bool withCoords;

};


} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_GEOMETRYFACTORY_H
