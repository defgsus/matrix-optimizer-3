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
    : GeometryModifier("Tesselate", QObject::tr("tesselate")),
      level_    (1)
{

}

void GeometryModifierTesselate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotess", 1);

    io << level_;
}

void GeometryModifierTesselate::deserialize(IO::DataStream &io)
{
    io.readHeader("geotess", 1);

    io >> level_;
}


void GeometryModifierTesselate::execute(Geometry *g)
{
    g->tesselate(level_);
}


} // namespace GEOM
} // namespace MO
