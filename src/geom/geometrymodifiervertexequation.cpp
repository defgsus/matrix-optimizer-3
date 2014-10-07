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
      equ_      ("x = x;\ny = y;\nz = z")
{

}

QString GeometryModifierVertexEquation::statusTip() const
{
    return QObject::tr("Applies a mathematical equation to each vertex");
}


void GeometryModifierVertexEquation::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoequvert", 2);

    io << equ_;
}

void GeometryModifierVertexEquation::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("geoequvert", 2);

    if (ver<2)
    {
        QString equx, equy, equz;
        io >> equx >> equy >> equz;
        equ_ = "x = " + equx + ";\ny = " + equy + ";\nz = " + equz;
    }
    else
        io >> equ_;
}

void GeometryModifierVertexEquation::execute(Geometry *g)
{
    g->transformWithEquation(equ_);
}

} // namespace GEOM
} // namespace MO
