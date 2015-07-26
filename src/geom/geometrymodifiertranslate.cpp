/** @file geometrymodifiertranslate.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiertranslate.h"
#include "geometry.h"
#include "types/properties.h"
#include "io/datastream.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierTranslate)

GeometryModifierTranslate::GeometryModifierTranslate()
    : GeometryModifier("Translate", QObject::tr("translate")),
      x_        (0.0),
      y_        (0.0),
      z_        (0.0)
{
    properties().set("tx", QObject::tr("x"),
                     QObject::tr("Translation on x axis"),
                     0.0, 0.1);
    properties().set("ty", QObject::tr("y"),
                     QObject::tr("Translation on y axis"),
                     0.0, 0.1);
    properties().set("tz", QObject::tr("z"),
                     QObject::tr("Translation on z axis"),
                     0.0, 0.1);
}

QString GeometryModifierTranslate::statusTip() const
{
    return QObject::tr("Moves the model around");
}


void GeometryModifierTranslate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotrans", 1);

    io << x_ << y_ << z_;
}

void GeometryModifierTranslate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geotrans", 1);

    Float x, y, z;
    io >> x >> y >> z;
    properties().set("x", x);
    properties().set("y", y);
    properties().set("z", z);
}


void GeometryModifierTranslate::execute(Geometry *g)
{
    g->translate(
            properties().get("x").toFloat(),
            properties().get("y").toFloat(),
            properties().get("z").toFloat());
}

} // namespace GEOM
} // namespace MO
