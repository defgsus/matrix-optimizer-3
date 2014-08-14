/** @file geometrymodifierscale.cpp

    @brief scales a geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifierscale.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierScale)

GeometryModifierScale::GeometryModifierScale()
    : GeometryModifier("Scale", QObject::tr("scale")),
      all_      (1.0),
      x_        (1.0),
      y_        (1.0),
      z_        (1.0)
{

}

void GeometryModifierScale::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoscale", 1);

    io << all_ << x_ << y_ << z_;
}

void GeometryModifierScale::deserialize(IO::DataStream &io)
{
    io.readHeader("geoscale", 1);

    io >> all_ >> x_ >> y_ >> z_;
}

void GeometryModifierScale::execute(Geometry *g)
{
    g->scale(all_ * x_, all_ * y_, all_ * z_);
}

} // namespace GEOM
} // namespace MO
