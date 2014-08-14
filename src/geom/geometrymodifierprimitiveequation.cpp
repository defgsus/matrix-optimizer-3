/** @file geometrymodifierprimitiveequation.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifierprimitiveequation.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierPrimitiveEquation)

GeometryModifierPrimitiveEquation::GeometryModifierPrimitiveEquation()
    : GeometryModifier("PrimitiveEquation", QObject::tr("primitive equation"))
{

}

void GeometryModifierPrimitiveEquation::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoequprim", 1);

    io << equX_ << equY_ << equZ_;
}

void GeometryModifierPrimitiveEquation::deserialize(IO::DataStream &io)
{
    io.readHeader("geoequprim", 1);

    io >> equX_ >> equY_ >> equZ_;
}

void GeometryModifierPrimitiveEquation::execute(Geometry *g)
{
    g->transformPrimitivesWithEquation(equX_, equY_, equZ_);
}

} // namespace GEOM
} // namespace MO
