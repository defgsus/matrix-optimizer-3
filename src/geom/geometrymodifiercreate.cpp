/** @file geometrymodifiercreate.cpp

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiercreate.h"
#include "io/datastream.h"
#include "geometry.h"
#include "geometryfactory.h"
#include "objloader.h"
#include "shploader.h"
#include "tool/deleter.h"
#include "io/filemanager.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierCreate)

Properties::NamedValues GeometryModifierCreate::namedTypes()
{
    Properties::NamedValues val;
    val.set("file", QObject::tr("Wavefront Object (obj)"), T_FILE_OBJ);
#ifndef MO_DISABLE_SHP
    val.set("shp", QObject::tr("Shapefile (shp)"), T_FILE_OBJ);
#endif
    val.set("quad", QObject::tr("quad"), T_QUAD);
    val.set("tetra", QObject::tr("tetrahedron"), T_TETRAHEDRON);
    val.set("hexa", QObject::tr("hexahedron (cube)"), T_BOX);
    val.set("hexauv", QObject::tr("hexahedron (cube) (full uv-map)"), T_BOX_UV);
    val.set("octa", QObject::tr("octahedron"), T_OCTAHEDRON);
    val.set("icosa", QObject::tr("icosahedron"), T_ICOSAHEDRON);
    val.set("dodeca", QObject::tr("dodecahedron"), T_DODECAHEDRON);
    val.set("cyl", QObject::tr("cylinder"), T_CYLINDER_CLOSED);
    val.set("cylo", QObject::tr("cylinder (open)"), T_CYLINDER_OPEN);
    val.set("cone", QObject::tr("cone"), T_CONE_CLOSED);
    val.set("coneo", QObject::tr("cone (open)"), T_CONE_OPEN);
    val.set("torus", QObject::tr("torus"), T_TORUS);
    val.set("uvsphere", QObject::tr("uv-sphere"), T_UV_SPHERE);
    val.set("gridxz", QObject::tr("coordinate system"), T_GRID_XZ);
    val.set("pgrid", QObject::tr("point grid"), T_POINT_GRID);
    val.set("lgrid", QObject::tr("line grid"), T_LINE_GRID);
    val.set("qgrid", QObject::tr("quad grid"), T_QUAD_GRID);
    return val;
};

const QStringList GeometryModifierCreate::typeIds =
{
    "file",
#ifndef MO_DISABLE_SHP
    "shp",
#endif
    "quad",
    "tetra", "hexa", "hexauv", "octa", "icosa", "dodeca",
    "cyl", "cylo", "cone", "coneo", "torus", "uvsphere",
    "gridxz", "pgrid", "lgrid", "qgrid"
};

GeometryModifierCreate::GeometryModifierCreate()
    : GeometryModifier  ("Create", QObject::tr("create"))
{
    properties().set("type", QObject::tr("type"),
                     QObject::tr("The type of geometry, source or file"),
                     namedTypes(), int(T_BOX_UV));

    properties().set("filename-obj", QObject::tr("obj filename"),
                     QObject::tr("The Wavefront object file to load"),
                     QString());
    properties().setSubType("filename-obj",
                            Properties::ST_FILENAME | IO::FT_MODEL);

    properties().set("filename-shp", QObject::tr("shp filename"),
                     QObject::tr("The shapefile to load"),
                     QString());
    properties().setSubType("filename-shp",
                            Properties::ST_FILENAME | IO::FT_SHAPEFILE);

    properties().set("asTriangles", QObject::tr("create triangles"),
                     QObject::tr("Selects lines or triangles"),
                     true);
    properties().set("shared", QObject::tr("shared vertices"),
                     QObject::tr("Minimizes the amount of vertices by "
                                 "reusing the same for adjacent primitives"),
                     true);
    properties().set("radius", QObject::tr("radius"),
                     QObject::tr("The radius of the object to create"),
                     true);

    properties().set("segments", QObject::tr("number segments"),
                     QObject::tr("Number of segments on x, y, z"),
                     QVector<uint>() << 10 << 10 << 1);

    properties().set("color", QObject::tr("ambient color"),
                     QObject::tr("The base color of the geometry"),
                     QVector<Float>() << .5f << .5f << .5f << 1.f,
                     QVector<Float>() << 0.f << 0.f << 0.f << 0.f,
                     QVector<Float>() << 1.f << 1.f << 1.f << 1.f,
                     QVector<Float>() << .1f << .1f << .1f << .1f);
}

bool GeometryModifierCreate::isFile() const
{
    Type t = (Type)properties().get("type").toInt();
    return t == T_FILE_OBJ || t == T_FILE_SHP;
}

QString GeometryModifierCreate::filename() const
{
    Type t = (Type)properties().get("type").toInt();
    return t == T_FILE_SHP
            ? properties().get("filename-shp").toString()
            : properties().get("filename-obj").toString();
}


QString GeometryModifierCreate::statusTip() const
{
    return QObject::tr("Creates a new geometry");
}

void GeometryModifierCreate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geocreate", 2);
}

void GeometryModifierCreate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    int ver = io.readHeader("geocreate", 2);

    if (ver < 2)
    {
        Type type;
        QString filename;
        bool asTriangles, sharedVertices;
        Float colorR, colorG, colorB, colorA, smallRadius;
        uint segmentsX, segmentsY, segmentsZ;

        io.readEnum(type, T_BOX_UV, typeIds);
        io >> filename >> asTriangles >> sharedVertices
           >> colorR >> colorG >> colorB >> colorA
           >> segmentsX >> segmentsY >> segmentsZ
           >> smallRadius;

        properties().set("type", int(type));
        if (filename.endsWith("obj"))
            properties().set("filename-obj", filename);
        else
            properties().set("filename-shp", filename);
        properties().set("asTriangles", asTriangles);
        properties().set("shared", sharedVertices);
        properties().set("radius", smallRadius);
        properties().set("color", QVector<Float>()
                         << colorR << colorG << colorB << colorA);
        properties().set("segments" , QVector<uint>()
                         << segmentsX << segmentsY << segmentsZ);
    }
}

void GeometryModifierCreate::execute(Geometry * g)
{
    const bool
            sharedVertices = properties().get("shared").toBool(),
            asTriangles = properties().get("asTriangles").toBool();
    const QVector<Float>
            color = properties().get("color").value<QVector<Float>>();
    const QVector<uint>
            segments = properties().get("segments").value<QVector<uint>>();
    const Float
            smallRadius = properties().get("radius").toFloat();
    const Type
            type = (Type)properties().get("type").toInt();


    // shared vertices?
    g->setSharedVertices(sharedVertices);

    // initial color
    g->setColor(color[0], color[1], color[2], color[3]);

    // create mesh
    switch (type)
    {
    case T_FILE_OBJ:
        if (!properties().get("filename-obj").toString().isEmpty())
            ObjLoader::getGeometry(IO::fileManager().localFilename(
                            properties().get("filename-obj").toString()), g);
    break;

#ifndef MO_DISABLE_SHP
    case T_FILE_SHP:
        if (!properties().get("filename-shp").toString().isEmpty())
            ShpLoader::getGeometry(IO::fileManager().localFilename(
                    properties().get("filename-shp").toString()), g,
                                   [=](double p){ setProgress(p); });
    break;
#endif

    case T_QUAD:
        GeometryFactory::createQuad(g, 1.f, 1.f, asTriangles);
    break;

    case T_BOX:
        GeometryFactory::createCube(g, 1.f, asTriangles);
    break;

    case T_BOX_UV:
        GeometryFactory::createTexturedBox(g, 1.f, 1.f, 1.f);
        if (!asTriangles)
            g->convertToLines();
    break;

    case T_GRID_XZ:
        GeometryFactory::createGridXZ(g, segments[0], segments[1], true);
    break;

    case T_LINE_GRID:
        GeometryFactory::createLineGrid(g, segments[0], segments[1], segments[2]);
    break;

    case T_POINT_GRID:
        GeometryFactory::createPointGrid(g, segments[0], segments[1], segments[2]);
    break;

    case T_QUAD_GRID:
        GeometryFactory::createQuadGrid(g, segments[0], segments[1], segments[2]);
    break;

    case T_UV_SPHERE:
        GeometryFactory::createUVSphere(g, 1.f, std::max((uint)3, segments[0]),
                               std::max((uint)2, segments[1]), asTriangles);
    break;

    case T_TETRAHEDRON:
        GeometryFactory::createTetrahedron(g, 1.f, asTriangles);
    break;

    case T_OCTAHEDRON:
        GeometryFactory::createOctahedron(g, 1.f, asTriangles);
    break;

    case T_ICOSAHEDRON:
        GeometryFactory::createIcosahedron(g, 1.f, asTriangles);
    break;

    case T_DODECAHEDRON:
        GeometryFactory::createDodecahedron(g, 1.f, asTriangles);
    break;

    case T_CYLINDER_CLOSED:
        GeometryFactory::createCylinder(g, 1.f, 1.f, segments[0], segments[1], false, asTriangles);
    break;

    case T_CYLINDER_OPEN:
        GeometryFactory::createCylinder(g, 1.f, 1.f, segments[0], segments[1], true, asTriangles);
    break;

    case T_CONE_CLOSED:
        GeometryFactory::createCone(g, 1.f, 1.f, segments[0], false, asTriangles);
    break;

    case T_CONE_OPEN:
        GeometryFactory::createCone(g, 1.f, 1.f, segments[0], true, asTriangles);
    break;

    case T_TORUS:
        GeometryFactory::createTorus(g, 1.f, smallRadius, segments[0], segments[1], asTriangles);
    break;

    }

    // unshared-vertices for non-files
    if (!sharedVertices && type != T_FILE_OBJ && type != T_FILE_SHP)
        g->unGroupVertices();
}


} // namespace GEOM
} // namespace MO
