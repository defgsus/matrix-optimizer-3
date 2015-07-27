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
    /*QStringList vars = {
            "x", "y", "z", "nx", "ny", "nz", "s", "t", "i", "p",
            "red", "green", "blue", "alpha", "bright",
            "x1", "y1", "z1", "x2", "y2", "z2", "x3", "y3", "z3",
            "nx1", "ny1", "nz1", "nx2", "ny2", "nz2", "nx3", "ny3", "nz3",
            "s1", "t1", "s2", "t2", "s3", "t3",
            "red1", "green1", "blue1", "alpha1",
            "red2", "green2", "blue2", "alpha2",
            "red3", "green3", "blue3", "alpha3" };
    */
    properties().set(
        "equ", QObject::tr("equation"),
        QObject::tr("The equation to apply to all the verices per primitive"),
        QString("x = x;\ny = y;\nz = z"));
}

QString GeometryModifierPrimitiveEquation::statusTip() const
{
    return QObject::tr("Applies an equation to each coordinate of the vertices of each primitive");
}


void GeometryModifierPrimitiveEquation::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoequprim", 3);
}

void GeometryModifierPrimitiveEquation::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("geoequprim", 3);

    if (ver < 3)
    {
        QString equ;
        if (ver<2)
        {
            QString equx, equy, equz;
            io >> equx >> equy >> equz;
            if (equx.isEmpty())
                equx = "x";
            if (equy.isEmpty())
                equy = "y";
            if (equz.isEmpty())
                equz = "z";
            equ = "x = " + equx + ";\ny = " + equy + ";\nz = " + equz;
        }
        else
            io >> equ;
        properties().set("equ", equ);
    }
}

void GeometryModifierPrimitiveEquation::execute(Geometry *g)
{
    g->transformPrimitivesWithEquation(
                properties().get("equ").toString());
}

} // namespace GEOM
} // namespace MO
