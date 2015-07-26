/** @file geometrymodifiernormals.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiernormals.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierNormals)

GeometryModifierNormals::GeometryModifierNormals()
    : GeometryModifier("Normals", QObject::tr("normal modification")),
      calc_     (true),
      invert_   (false)
{
    properties().set(
        "tri", QObject::tr("from triangles"),
        QObject::tr("Automatically calculates the normals for the triangles"),
        true);
    properties().set(
        "invert", QObject::tr("invert"),
        QObject::tr("Inverts normals, so that they point into the opposite direction"),
        false);
}

QString GeometryModifierNormals::statusTip() const
{
    return QObject::tr("Manipulates normals or calculates the normals from given triangles");
}


void GeometryModifierNormals::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geonormal", 1);

    io << calc_ << invert_;
}

void GeometryModifierNormals::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geonormal", 1);

    bool calc, invert;
    io >> calc >> invert;
    properties().set("tri", calc);
    properties().set("invert", invert);
}


void GeometryModifierNormals::execute(Geometry *g)
{
    if (properties().get("tri").toBool())
        g->calculateTriangleNormals();
    if (properties().get("invert").toBool())
        g->invertNormals();
}

} // namespace GEOM
} // namespace MO
