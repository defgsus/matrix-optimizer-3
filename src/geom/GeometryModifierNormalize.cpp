/** @file geometrymodifiernormalize.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "GeometryModifierNormalize.h"
#include "Geometry.h"
#include "types/Properties.h"
#include "io/DataStream.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierNormalize)

GeometryModifierNormalize::GeometryModifierNormalize()
    : GeometryModifier("Normalize", QObject::tr("normalize vertices"))
{
    properties().set(
        "amt", QObject::tr("amount"),
        QObject::tr("The amount of how much the normalization is applied [0,1]"),
        1.0f, 0.05f);
}

QString GeometryModifierNormalize::statusTip() const
{
    return QObject::tr("Normalizes all vertices - that is, "
                       "all points are made to lie on a sphere around the origin");
}


void GeometryModifierNormalize::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geonorm", 2);
}

void GeometryModifierNormalize::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("geonorm", 2);

    if (ver < 2)
    {
        Float n;
        io >> n;
        properties().set("amt", n);
    }
}


void GeometryModifierNormalize::execute(Geometry *g)
{
    g->normalizeSphere(1, properties().get("amt").toFloat());
}

} // namespace GEOM
} // namespace MO
