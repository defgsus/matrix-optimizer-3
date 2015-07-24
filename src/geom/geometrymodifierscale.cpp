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
    properties().set(
        "all", QObject::tr("overall scale"),
        QObject::tr("The scale on all three axis"),
        1.0, 0.1);
    properties().set(
        "x", QObject::tr("x scale"),
        QObject::tr("The scale on the x axis"),
        1.0, 0.1);
    properties().set(
        "y", QObject::tr("y scale"),
        QObject::tr("The scale on the y axis"),
        1.0, 0.1);
    properties().set(
        "z", QObject::tr("z scale"),
        QObject::tr("The scale on the z axis"),
        1.0, 0.1);
}

QString GeometryModifierScale::statusTip() const
{
    return QObject::tr("Scales the vertices and therefore the whole model");
}


void GeometryModifierScale::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoscale", 1);

    io << all_ << x_ << y_ << z_;
}

void GeometryModifierScale::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geoscale", 1);

    Float all, x, y, z;
    io >> all >> x >> y >> z;
    properties().set("all", all);
    properties().set("x", x);
    properties().set("y", y);
    properties().set("z", z);
}

void GeometryModifierScale::execute(Geometry *g)
{
    const Float
            all = properties().get("all").toDouble(),
            x = all * properties().get("x").toDouble(),
            y = all * properties().get("y").toDouble(),
            z = all * properties().get("z").toDouble();
    g->scale(x, y, z);
}

} // namespace GEOM
} // namespace MO
