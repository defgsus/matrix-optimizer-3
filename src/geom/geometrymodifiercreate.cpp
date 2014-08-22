/** @file geometrymodifiercreate.cpp

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiercreate.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

//MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierCreate)

const QStringList GeometryModifierCreate::typeIds =
{
    "file", "quad",
    "tetra", "hexa", "octa", "icosa", "dodeca",
    "cyl", "cylo", "torus", "uvsphere",
    "gridxz", "lgrid"
};

const QStringList GeometryModifierCreate::typeNames =
{
    QObject::tr("file"),
    QObject::tr("quad"),
    QObject::tr("tetrahedron"),
    QObject::tr("hexahedron (cube)"),
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
    : GeometryModifier("Create", QObject::tr("create")),
      type_     (T_BOX)
{

}

QString GeometryModifierCreate::statusTip() const
{
    return QObject::tr("Creates new geometry XXX not working yet");
}

void GeometryModifierCreate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geocreate", 1);

    io << typeIds[type_];
    io << filename_ << asTriangles_ << sharedVertices_;
}

void GeometryModifierCreate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geocreate", 1);

    io.readEnum(type_, T_BOX, typeIds);
    io >> filename_ >> asTriangles_ >> sharedVertices_;
}

void GeometryModifierCreate::execute(Geometry *g)
{
#if (0)
    // XXX
    g->setSharedVertices(sharedVertices_);

    // initial color
    g->setColor(1,1,1,1);

    // create mesh
    switch (type_)
    {
    // XXX
    case T_FILE:
        if (!filename_.isEmpty() && loader_)
        {
            loader_->loadFile(filename_);
            loader_->getGeometry(g);
        }
    break;
    case T_QUAD:
        createQuad(g, 1.f, 1.f, set->asTriangles);
    break;

    case T_BOX:
        createCube(g, 1.f, set->asTriangles);
    break;

    case T_GRID_XZ:
        createGridXZ(g, set->segmentsX, set->segmentsY, set->withCoords);
    break;

    case T_LINE_GRID:
        createLineGrid(g, set->segmentsX, set->segmentsY, set->segmentsZ);
    break;

    case T_UV_SPHERE:
        createUVSphere(g, 1.f, std::max((uint)3, set->segmentsX),
                               std::max((uint)2, set->segmentsY), set->asTriangles);
    break;

    case T_TETRAHEDRON:
        createTetrahedron(g, 1.f, set->asTriangles);
    break;

    case T_OCTAHEDRON:
        createOctahedron(g, 1.f, set->asTriangles);
    break;

    case T_ICOSAHEDRON:
        createIcosahedron(g, 1.f, set->asTriangles);
    break;

    case T_DODECAHEDRON:
        createDodecahedron(g, 1.f, set->asTriangles);
    break;

    case T_CYLINDER_CLOSED:
        createCylinder(g, 1.f, 1.f, set->segmentsX, set->segmentsY, false, set->asTriangles);
    break;

    case T_CYLINDER_OPEN:
        createCylinder(g, 1.f, 1.f, set->segmentsX, set->segmentsY, true, set->asTriangles);
    break;

    case T_TORUS:
        createTorus(g, 1.f, set->smallRadius, set->segmentsX, set->segmentsY, set->asTriangles);
    break;

    }
#endif
}


} // namespace GEOM
} // namespace MO
