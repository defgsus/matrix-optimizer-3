/** @file geometrymodifiertranslate.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "GeometryModifierTranslate.h"
#include "Geometry.h"
#include "types/Properties.h"
#include "io/DataStream.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierTranslate)

GeometryModifierTranslate::GeometryModifierTranslate()
    : GeometryModifier("Translate", QObject::tr("translate"))
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

    io.writeHeader("geotrans", 2);
}

void GeometryModifierTranslate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("geotrans", 2);

    if (ver < 2)
    {
        Float x, y, z;
        io >> x >> y >> z;
        properties().set("tx", x);
        properties().set("ty", y);
        properties().set("tz", z);
    }
}


void GeometryModifierTranslate::execute(Geometry *g)
{
    g->translate(
            properties().get("tx").toFloat(),
            properties().get("ty").toFloat(),
            properties().get("tz").toFloat());
}

} // namespace GEOM
} // namespace MO
