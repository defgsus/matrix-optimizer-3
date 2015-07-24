/** @file geometrymodifiernormalize.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiernormalize.h"
#include "geometry.h"
#include "types/properties.h"
#include "io/datastream.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierNormalize)

GeometryModifierNormalize::GeometryModifierNormalize()
    : GeometryModifier("Normalize", QObject::tr("normalize vertices")),
      n_      (1.0)
{
    properties().set(
        "amt", QObject::tr("amount"),
        QObject::tr("The amount of how much the normalization is applied"),
        1.0, 0., 1., 0.05);
}

QString GeometryModifierNormalize::statusTip() const
{
    return QObject::tr("Normalizes all vertices - that is, "
                       "all points are made to lie on a sphere around the origin");
}


void GeometryModifierNormalize::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geonorm", 1);

    io << n_;
}

void GeometryModifierNormalize::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geonorm", 1);

    io >> n_;
}


void GeometryModifierNormalize::execute(Geometry *g)
{
    g->normalizeSphere(1, n_);
}

} // namespace GEOM
} // namespace MO
