/** @file geometrymodifierenum.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.05.2015</p>
*/

#include "GeometryModifierEnum.h"
#include "io/DataStream.h"
#include "Geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierEnum)

GeometryModifierEnum::GeometryModifierEnum()
    : GeometryModifier("Enum", QObject::tr("enumeration attribute"))
{
    properties().set(
        "indexName", QObject::tr("attribute name"),
        QObject::tr("The name of the attribute (as used in glsl)"),
        QString("a_index"));

    properties().set(
        "doIndex", QObject::tr("vertex index"),
        QObject::tr("Adds the zero-based index to each vertex"),
        true);
}

QString GeometryModifierEnum::statusTip() const
{
    return QObject::tr("Enumerates all vertices and stores the information in a user attribute");
}


void GeometryModifierEnum::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoenum", 2);
}

void GeometryModifierEnum::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("geoenum", 2);

    if (ver < 2)
    {
        bool doIndex;
        QString indexName;
        io >> doIndex >> indexName;
        properties().set("doIndex", doIndex);
        properties().set("indexName", indexName);
    }
}


void GeometryModifierEnum::execute(Geometry *g)
{
    if (properties().get("doIndex").toBool())
        g->addEnumerationAttribute(
                    properties().get("indexName").toString());
}


} // namespace GEOM
} // namespace MO
