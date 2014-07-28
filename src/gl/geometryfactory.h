/** @file geometryfactory.h

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#ifndef MOSRC_GL_GEOMETRYFACTORY_H
#define MOSRC_GL_GEOMETRYFACTORY_H

#include <QStringList>

#include "types/int.h"

namespace MO {
namespace IO { class DataStream; }
namespace GL {

class Geometry;
class GeometryFactorySettings;

class GeometryFactory
{
public:

    static void createFromSettings(Geometry *, const GeometryFactorySettings*);

    static void createQuad(Geometry *, float side_length_x, float side_length_y,
                           bool asTriangles = true);

    static void createCube(Geometry *, float side_length, bool asTriangles = true);
    static void createBox(Geometry *,
            float side_length_x, float side_length_y, float side_length_z,
                          bool asTriangles = true);

    static void createUVSphere(Geometry *, float rad, uint segu, uint segv,
                               bool asTriangles = true);
    static void createUVSphereLines(Geometry *, float rad, uint segu, uint segv);

    static void createGrid(Geometry *, int sizeU, int sizeV, bool with_coordinate_system);
};



class GeometryFactorySettings
{
public:
    enum Type
    {
        T_QUAD,
        T_BOX,
        T_GRID,
        T_UV_SPHERE
    };
    static const uint numTypes = T_UV_SPHERE + 1;
    static const QStringList typeIds;
    static const QStringList typeNames;

    GeometryFactorySettings();

    // ---- public member ----

    Type type;
    bool asTriangles, convertToLines, sharedVertices;
    float colorR, colorG, colorB, colorA;
    float scale, scaleX, scaleY, scaleZ;
    uint gridSize, segmentsU, segmentsV;
    bool withCoords;
};


} // namespace GL
} // namespace MO


#endif // MOSRC_GL_GEOMETRYFACTORY_H
