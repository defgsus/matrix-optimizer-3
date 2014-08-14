/** @file geometrymodifiertranslate.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiertranslate.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierTranslate)

GeometryModifierTranslate::GeometryModifierTranslate()
    : GeometryModifier("Translate", QObject::tr("translate")),
      x_        (1.0),
      y_        (1.0),
      z_        (1.0)
{

}

void GeometryModifierTranslate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotrans", 1);

    io << x_ << y_ << z_;
}

void GeometryModifierTranslate::deserialize(IO::DataStream &io)
{
    io.readHeader("geotrans", 1);

    io >> x_ >> y_ >> z_;
}


void GeometryModifierTranslate::execute(Geometry *g)
{
    g->translate(x_, y_, z_);
}

} // namespace GEOM
} // namespace MO
