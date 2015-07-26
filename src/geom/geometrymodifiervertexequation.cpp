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
/*    QStringList vars = {
        "x", "y", "z", "i", "s", "t",
        "red", "green", "blue", "alpha", "bright" };
*/
    properties().set(
        "equ", QObject::tr("equation"),
        QObject::tr("The equation to apply to all the verices"),
        QString("x = x;\ny = y;\nz = z"));
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

void GeometryModifierVertexEquation::execute(Geometry *g)
{
    g->transformWithEquation(
                properties().get("equ").toString()
                );
}

} // namespace GEOM
} // namespace MO
