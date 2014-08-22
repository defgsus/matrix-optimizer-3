/** @file geometrymodifiervertexequation.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiervertexequation.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierVertexEquation)

GeometryModifierVertexEquation::GeometryModifierVertexEquation()
    : GeometryModifier("VertexEquation", QObject::tr("vertex equation")),
      equX_     ("x"),
      equY_     ("y"),
      equZ_     ("z")
{

}

QString GeometryModifierVertexEquation::statusTip() const
{
    return QObject::tr("Applies a mathematical equation to each vertex");
}


void GeometryModifierVertexEquation::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoequvert", 1);

    io << equX_ << equY_ << equZ_;
}

void GeometryModifierVertexEquation::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geoequvert", 1);

    io >> equX_ >> equY_ >> equZ_;
}

void GeometryModifierVertexEquation::execute(Geometry *g)
{
    g->transformWithEquation(equX_, equY_, equZ_);
}

} // namespace GEOM
} // namespace MO
