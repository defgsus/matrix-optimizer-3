
#include "geometrymodifiertext.h"
#include "geom/textmesh.h"
#include "io/datastream.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierText)

GeometryModifierText::GeometryModifierText()
    : GeometryModifier  ("Text", QObject::tr("Text"))
{
    TextMesh tm;
    properties() = tm.properties();
}

QString GeometryModifierText::statusTip() const
{
    return QObject::tr("Create a Text as geometry");
}

void GeometryModifierText::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotext", 1);
}

void GeometryModifierText::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geotext", 1);
}

void GeometryModifierText::execute(Geometry * g)
{
    TextMesh tm;
    tm.setProperties(properties());
    tm.getGeometry(g);
}


} // namespace GEOM
} // namespace MO

