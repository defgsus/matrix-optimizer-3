/** @file geometrymodifierconvertlines.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "GeometryModifierConvertlines.h"
#include "io/DataStream.h"
#include "Geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierConvertLines)

GeometryModifierConvertLines::GeometryModifierConvertLines()
    : GeometryModifier("ConvertLines", QObject::tr("convert to lines"))
{

}

QString GeometryModifierConvertLines::statusTip() const
{
    return QObject::tr("Converts triangles to lines");
}

void GeometryModifierConvertLines::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoconvline", 1);
}

void GeometryModifierConvertLines::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geoconvline", 1);
}

void GeometryModifierConvertLines::execute(Geometry *g)
{
    g->convertToLines();
}

} // namespace GEOM
} // namespace MO
