/** @file geometrymodifiertesselate.cpp

    @brief Tesselates a Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiertesselate.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierTesselate)

GeometryModifierTesselate::GeometryModifierTesselate()
    : GeometryModifier("Tesselate", QObject::tr("tesselate"))
{
    properties().set(
        "level", QObject::tr("triangulation level"),
        QObject::tr("Number of successive tesselations"),
        1u, 1u, 20u);
    properties().set(
        "minArea", QObject::tr("minimum area"),
        QObject::tr("The smallest triangle area that will be considered for tesselation"),
        0.0f, 0.1f);
    properties().setMin("minArea", 0.f);
    properties().set(
        "minLength", QObject::tr("minimum side-length"),
        QObject::tr("The length of a triangle vertex that will be considered for tesselation"),
        0.0f, 0.1f);
    properties().setMin("minLength", 0.f);
}

QString GeometryModifierTesselate::statusTip() const
{
    return QObject::tr("Sub-triangulates triangles to a given level - "
                       "be careful, size of model grows exponetially!");
}


void GeometryModifierTesselate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotess", 3);
}

void GeometryModifierTesselate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    int ver = io.readHeader("geotess", 3);

    if (ver < 3)
    {
        uint level;
        Float minArea=0, minLength=0;
        io >> level;
        if (ver >= 2)
            io >> minArea >> minLength;
        properties().set("level", level);
        properties().set("minArea", minArea);
        properties().set("minLength", minLength);
    }
}


void GeometryModifierTesselate::execute(Geometry *g)
{
    g->tesselateTriangles(
                properties().get("minArea").toFloat(),
                properties().get("minLength").toFloat(),
                properties().get("level").toUInt());
}


} // namespace GEOM
} // namespace MO
