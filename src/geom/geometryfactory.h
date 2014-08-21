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

#include "geometrymodifierchain.h"

namespace MO {
namespace IO { class DataStream; }
namespace GEOM {

class Geometry;
class GeometryFactorySettings;
class GeometryModifierChain;
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

    /** Creates a correctly uv-mapped box */
    static void createTexturedBox(Geometry *,
            Float side_length_x, Float side_length_y, Float side_length_z);

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
    static void createLineGrid(Geometry *, int sizeX, int sizeY, int sizeZ);

};



} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_GEOMETRYFACTORY_H
