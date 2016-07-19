/** @file geometrymodifierextrude.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "GeometryModifierExtrude.h"
#include "io/DataStream.h"
#include "Geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierExtrude)

GeometryModifierExtrude::GeometryModifierExtrude()
    : GeometryModifier("Extrude", QObject::tr("extrude faces"))
{
    properties().set(
        "const", QObject::tr("extrusion by constant"),
        QObject::tr("Extrudes triangles along their normal by a constant value"),
        0.1f, 0.05f);
    properties().set(
        "factor", QObject::tr("extrusion by factor"),
        QObject::tr("Extrudes triangles vertices along their normal by a factor "
                    "of the length of adjecent edges"),
        0.0f, 0.05f);
    properties().set(
        "shiftCenter", QObject::tr("center shift"),
        QObject::tr("Shifts the extruded vertices towards the center of the triangles"),
        0.0f, 0.05f);
    properties().set(
        "doFaces", QObject::tr("create orthogonal faces"),
        QObject::tr("Enables the creation of the outside faces, "
                    "orthogonal to extruded faces"),
        true);
    properties().set(
        "doCheckEdge", QObject::tr("recognize edges"),
        QObject::tr("If e.g. two triangles are recognized as forming a quad, "
                    "no orthogonal face is created for the inner edge"),
        false);


}

QString GeometryModifierExtrude::statusTip() const
{
    return QObject::tr("Extrudes triangles - "
                       "that is, all surfaces are shifted along their normals "
                       "and side faces are created");
}


void GeometryModifierExtrude::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoextrude", 4);
}

void GeometryModifierExtrude::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("geoextrude", 4);

    if (ver < 4)
    {
        Float constant, factor, shiftCenter = 0.f;
        bool doOuterFaces = true, doRecogEdges = false;

        io >> constant >> factor;

        if (ver>=2)
            io >> doOuterFaces >> doRecogEdges;
        if (ver>=3)
            io >> shiftCenter;

        properties().set("const", constant);
        properties().set("factor", factor);
        properties().set("shift", shiftCenter);
        properties().set("doFaces", doOuterFaces);
        properties().set("doCheckEdge", doRecogEdges);
    }
}

void GeometryModifierExtrude::execute(Geometry *g)
{
    if (!g->numTriangles())
        return;

    Geometry * geom = new Geometry();
    geom->setSharedVertices(g->sharedVertices(), g->sharedVerticesThreshold());

    g->extrudeTriangles(*geom,
                        properties().get("const").toFloat(),
                        properties().get("factor").toFloat(),
                        properties().get("shift").toFloat(),
                        properties().get("doFaces").toBool(),
                        properties().get("doCheckEdge").toBool());

    // explicitly ungroup vertices if so desired
    if (!g->sharedVertices())
        geom->unGroupVertices();

    *g = *geom;
    geom->releaseRef("GeometryModifierExtrude release temp");
}

} // namespace GEOM
} // namespace MO
