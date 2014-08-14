/** @file geometrymodifiernormalize.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiernormalize.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierNormalize)

GeometryModifierNormalize::GeometryModifierNormalize()
    : GeometryModifier("Normalize", QObject::tr("normalize vertices")),
      n_      (1.0)
{

}

void GeometryModifierNormalize::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geonorm", 1);

    io << n_;
}

void GeometryModifierNormalize::deserialize(IO::DataStream &io)
{
    io.readHeader("geonorm", 1);

    io >> n_;
}


void GeometryModifierNormalize::execute(Geometry *g)
{
    g->normalizeSphere(1, n_);
}

} // namespace GEOM
} // namespace MO
