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
    : GeometryModifier("PrimitiveEquation", QObject::tr("primitive equation")),
      equ_     ("x = x; y = y; z = z")
{

}

QString GeometryModifierPrimitiveEquation::statusTip() const
{
    return QObject::tr("Applies an equation to each coordinate of the vertices of each primitive");
}


void GeometryModifierPrimitiveEquation::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoequprim", 2);

    io << equ_;
}

void GeometryModifierPrimitiveEquation::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("geoequprim", 2);

    if (ver<2)
    {
        QString equx, equy, equz;
        io >> equx >> equy >> equz;
        equ_ = "x = " + equx + ";\ny = " + equy + ";\nz = " + equz;
    }
    else
        io >> equ_;
}

void GeometryModifierPrimitiveEquation::execute(Geometry *g)
{
    g->transformPrimitivesWithEquation(equ_);
}

} // namespace GEOM
} // namespace MO
