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

}

void GeometryModifierNormals::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geonormal", 1);

    io << calc_ << invert_;
}

void GeometryModifierNormals::deserialize(IO::DataStream &io)
{
    io.readHeader("geonormal", 1);

    io >> calc_ >> invert_;
}


void GeometryModifierNormals::execute(Geometry *g)
{
    if (calc_)
        g->calculateTriangleNormals();
    if (invert_)
        g->invertNormals();
}

} // namespace GEOM
} // namespace MO
