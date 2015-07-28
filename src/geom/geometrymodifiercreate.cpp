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
    val.set("file", QObject::tr("Wavefront Object (obj"), T_FILE_OBJ);
#ifndef MO_DISABLE_SHP
    val.set("shp", QObject::tr("Wavefront Object (obj"), T_FILE_OBJ);
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
/*
    QObject::tr("Wavefront Object (obj)"),
#ifndef MO_DISABLE_SHP
    QObject::tr("Shapefile (shp)"),
#endif
    QObject::tr("quad"),
    QObject::tr("tetrahedron"),
    QObject::tr("hexahedron (cube)"),
    QObject::tr("hexahedron (cube) uv-mapped"),
    QObject::tr("octahedron"),
    QObject::tr("icosahedron"),
    QObject::tr("dodecahedron"),
    QObject::tr("cylinder (closed)"),
    QObject::tr("cylinder (open)"),
    QObject::tr("cone (closed)"),
    QObject::tr("cone (open)"),
    QObject::tr("torus"),
    QObject::tr("uv-sphere"),
    QObject::tr("coordinate system"),
    QObject::tr("line-grid"),
    QObject::tr("point-grid"),
    QObject::tr("quad-grid")
};
*/

GeometryModifierCreate::GeometryModifierCreate()
    : GeometryModifier  ("Create", QObject::tr("create")),
      type_             (T_BOX_UV),
      asTriangles_      (true),
      sharedVertices_   (true),
      colorR_           (0.5),
      colorG_           (0.5),
      colorB_           (0.5),
      colorA_           (1.0),
      segmentsX_        (10),
      segmentsY_        (10),
      segmentsZ_        (1),
      smallRadius_      (0.3)
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

    properties().set("color", QObject::tr("ambient color"),
                     QObject::tr("The base color of the geometry"),
                     QVector<Float>() << .5f << .5f << .5f << 1.f,
                     QVector<Float>() << 0.f << 0.f << 0.f << 0.f,
                     QVector<Float>() << 1.f << 1.f << 1.f << 1.f,
                     QVector<Float>() << .1f << .1f << .1f << .1f);

    properties().set("segments", QObject::tr("number segments"),
                     QObject::tr("Number of segments on x, y, z"),
                     QVector<uint>() << 10 << 10 << 1);
}

QString GeometryModifierCreate::statusTip() const
{
    return QObject::tr("Creates a new geometry");
}

void GeometryModifierCreate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geocreate", 1);

    io << typeIds[type_];
    io << filename_ << asTriangles_ << sharedVertices_
       << colorR_ << colorG_ << colorB_ << colorA_
       << segmentsX_ << segmentsY_ << segmentsZ_
       << smallRadius_;
}

void GeometryModifierCreate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    int ver = io.readHeader("geocreate", 2);

    if (ver < 2)
    {
        io.readEnum(type_, T_BOX_UV, typeIds);
        io >> filename_ >> asTriangles_ >> sharedVertices_
           >> colorR_ >> colorG_ >> colorB_ >> colorA_
           >> segmentsX_ >> segmentsY_ >> segmentsZ_
           >> smallRadius_;
    }
}

void GeometryModifierCreate::execute(Geometry * g)
{
    //Geometry * g = new Geometry();
    //ScopedDeleter<Geometry> auto_remove(g);

    // shared vertices?
    g->setSharedVertices(sharedVertices_);

    // initial color
    g->setColor(colorR_, colorG_, colorB_, colorA_);

    // create mesh
    switch (type_)
    {
    case T_FILE_OBJ:
        if (!filename_.isEmpty())
            ObjLoader::getGeometry(IO::fileManager().localFilename(filename_), g);
    break;

#ifndef MO_DISABLE_SHP
    case T_FILE_SHP:
        if (!filename_.isEmpty())
            ShpLoader::getGeometry(IO::fileManager().localFilename(filename_), g,
                                   [=](double p){ setProgress(p); });
    break;
#endif

    case T_QUAD:
        GeometryFactory::createQuad(g, 1.f, 1.f, asTriangles_);
    break;

    case T_BOX:
        GeometryFactory::createCube(g, 1.f, asTriangles_);
    break;

    case T_BOX_UV:
        GeometryFactory::createTexturedBox(g, 1.f, 1.f, 1.f);
        if (!asTriangles())
            g->convertToLines();
    break;

    case T_GRID_XZ:
        GeometryFactory::createGridXZ(g, segmentsX_, segmentsY_, true);
    break;

    case T_LINE_GRID:
        GeometryFactory::createLineGrid(g, segmentsX_, segmentsY_, segmentsZ_);
    break;

    case T_POINT_GRID:
        GeometryFactory::createPointGrid(g, segmentsX_, segmentsY_, segmentsZ_);
    break;

    case T_QUAD_GRID:
        GeometryFactory::createQuadGrid(g, segmentsX_, segmentsY_, segmentsZ_);
    break;

    case T_UV_SPHERE:
        GeometryFactory::createUVSphere(g, 1.f, std::max((uint)3, segmentsX_),
                               std::max((uint)2, segmentsY_), asTriangles_);
    break;

    case T_TETRAHEDRON:
        GeometryFactory::createTetrahedron(g, 1.f, asTriangles_);
    break;

    case T_OCTAHEDRON:
        GeometryFactory::createOctahedron(g, 1.f, asTriangles_);
    break;

    case T_ICOSAHEDRON:
        GeometryFactory::createIcosahedron(g, 1.f, asTriangles_);
    break;

    case T_DODECAHEDRON:
        GeometryFactory::createDodecahedron(g, 1.f, asTriangles_);
    break;

    case T_CYLINDER_CLOSED:
        GeometryFactory::createCylinder(g, 1.f, 1.f, segmentsX_, segmentsY_, false, asTriangles_);
    break;

    case T_CYLINDER_OPEN:
        GeometryFactory::createCylinder(g, 1.f, 1.f, segmentsX_, segmentsY_, true, asTriangles_);
    break;

    case T_CONE_CLOSED:
        GeometryFactory::createCone(g, 1.f, 1.f, segmentsX_, false, asTriangles_);
    break;

    case T_CONE_OPEN:
        GeometryFactory::createCone(g, 1.f, 1.f, segmentsX_, true, asTriangles_);
    break;

    case T_TORUS:
        GeometryFactory::createTorus(g, 1.f, smallRadius_, segmentsX_, segmentsY_, asTriangles_);
    break;

    }

    // unshared-vertices for non-files
    if (!sharedVertices_ && type_ != T_FILE_OBJ)
        g->unGroupVertices();

    //geometry->addGeometry(*g);
}


} // namespace GEOM
} // namespace MO
