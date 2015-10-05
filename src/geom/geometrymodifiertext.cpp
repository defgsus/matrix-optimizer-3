
#include "geometrymodifiertext.h"
#include "geometry.h"
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
    properties().set("shared_vertices",
                     QObject::tr("shared vertices"),
                     QObject::tr("If enabled, vertices with same positions are reused"),
                     false);
    properties().set("color", QObject::tr("ambient color"),
                     QObject::tr("The base color of the geometry"),
                     QVector<Float>() << .5f << .5f << .5f << 1.f,
                     QVector<Float>() << 0.f << 0.f << 0.f << 0.f,
                     QVector<Float>() << 1.f << 1.f << 1.f << 1.f,
                     QVector<Float>() << .1f << .1f << .1f << .1f);
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
    auto color = properties().get("color").value<QVector<Float>>();
    g->setColor(color[0], color[1], color[2], color[3]);

    TextMesh tm;
    tm.setProperties(properties());
    tm.getGeometry(g, properties().get("shared_vertices").toBool());
}


} // namespace GEOM
} // namespace MO

