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
#include "tool/deleter.h"
#include "io/filemanager.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierCreate)

const QStringList GeometryModifierCreate::typeIds =
{
    "file", "quad",
    "tetra", "hexa", "hexauv", "octa", "icosa", "dodeca",
    "cyl", "cylo", "torus", "uvsphere",
    "gridxz", "lgrid"
};

const QStringList GeometryModifierCreate::typeNames =
{
    QObject::tr("file"),
    QObject::tr("quad"),
    QObject::tr("tetrahedron"),
    QObject::tr("hexahedron (cube)"),
    QObject::tr("hexahedron (cube) uv-mapped"),
    QObject::tr("octahedron"),
    QObject::tr("icosahedron"),
    QObject::tr("dodecahedron"),
    QObject::tr("cylinder (closed)"),
    QObject::tr("cylinder (open)"),
    QObject::tr("torus"),
    QObject::tr("uv-sphere"),
    QObject::tr("coordinate system"),
    QObject::tr("line-grid")
};

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
      smallRadius_      (0.1)
{

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

    io.readHeader("geocreate", 1);

    io.readEnum(type_, T_BOX_UV, typeIds);
    io >> filename_ >> asTriangles_ >> sharedVertices_
       >> colorR_ >> colorG_ >> colorB_ >> colorA_
       >> segmentsX_ >> segmentsY_ >> segmentsZ_
       >> smallRadius_;
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
    case T_FILE:
        if (!filename_.isEmpty())
            ObjLoader::getGeometry(IO::fileManager().localFilename(filename_), g);
    break;
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
        GeometryFactory::createGridXZ(g, segmentsX_, segmentsY_, false);
    break;

    case T_LINE_GRID:
        GeometryFactory::createLineGrid(g, segmentsX_, segmentsY_, segmentsZ_);
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

    case T_TORUS:
        GeometryFactory::createTorus(g, 1.f, smallRadius_, segmentsX_, segmentsY_, asTriangles_);
    break;

    }

    // unshared-vertices for non-files
    if (!sharedVertices_ && type_ != T_FILE)
        g->unGroupVertices();

    //geometry->addGeometry(*g);
}


} // namespace GEOM
} // namespace MO
