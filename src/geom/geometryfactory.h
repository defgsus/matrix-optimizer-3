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
#include "types/vector.h"

namespace MO {
namespace GEOM {

class Geometry;

/** Collection of static functions to add basic models to a Geometry class.

    All functions ADD data to exisiting geometries! */
class GeometryFactory
{
public:

    // --------------------- static functions ------------------

    static void createQuad(Geometry *, Float side_length_x, Float side_length_y,
                           bool asTriangles = true);

    static void createCube(Geometry *, Float side_length, bool asTriangles = true);
    static void createBox(Geometry *,
            Float side_length_x, Float side_length_y, Float side_length_z,
                          bool asTriangles = true, const Vec3 & offset = Vec3(0));

    /** Creates a 'correctly' uv-mapped box */
    static void createTexturedBox(Geometry *,
            Float side_length_x, Float side_length_y, Float side_length_z,
            const Vec3 & offset = Vec3(0));

    static void createUVSphere(Geometry *, Float rad, uint segu, uint segv,
                               bool asTriangles = true, const Vec3 & offset = Vec3(0));
    static void createUVSphereLines(Geometry *, Float rad, uint segu, uint segv);

    static void createDome(Geometry *, Float radius, Float coverage, uint segu, uint segv,
                           bool asTriangles);
    static void createDomeLines(Geometry *, Float radius, Float coverage, uint segu, uint segv);

    static void createCylinder(Geometry *, Float rad, Float height, uint segu, uint segv, bool open,
                               bool asTriangles);

    static void createCone(Geometry *, Float rad, Float height, uint seg, bool open,
                               bool asTriangles);

    static void createTorus(Geometry *, Float rad_outer, Float rad_inner, uint segu, uint segv,
                            bool asTriangles, const Vec3 & offset = Vec3(0));

    static void createTetrahedron(Geometry *, Float scale, bool asTriangles = true);
    static void createOctahedron(Geometry *, Float scale, bool asTriangles = true);
    static void createIcosahedron(Geometry *, Float scale, bool asTriangles = true);
    static void createDodecahedron(Geometry *, Float scale, bool asTriangles = true);

    static void createGridXZ(Geometry *, int sizeX, int sizeZ, bool with_coordinate_system);
    static void createLineGrid(Geometry *, int sizeX, int sizeY, int sizeZ);
    static void createPointGrid(Geometry *, int sizeX, int sizeY, int sizeZ);
    static void createQuadGrid(Geometry *, int sizeX, int sizeY, int sizeZ);

    /** mono-spaced line font [0,1] */
    static void createFont(Geometry *, const Mat4& matrix, uint16_t utf16);

    static void createText(Geometry *, Mat4 matrix, const QString& text);
};



} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_GEOMETRYFACTORY_H
