/** @file geometrymodifierenum.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.05.2015</p>
*/

#include "geometrymodifierenum.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierEnum)

GeometryModifierEnum::GeometryModifierEnum()
    : GeometryModifier("Enum", QObject::tr("enumeration attribute")),
      doIndex_      (true),
      indexName_    ("a_index")
{

}

QString GeometryModifierEnum::statusTip() const
{
    return QObject::tr("Enumerates all vertices and stores the information in a user attribute");
}


void GeometryModifierEnum::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoenum", 1);

    io << doIndex_ << indexName_;
}

void GeometryModifierEnum::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geoenum", 1);

    io >> doIndex_ >> indexName_;
}


void GeometryModifierEnum::execute(Geometry *g)
{
    if (doIndex_)
        g->addEnumerationAttribute(indexName_);
}


} // namespace GEOM
} // namespace MO
