/** @file geometrymodifier.cpp

    @brief Abstract base of Geometry modifiers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifier.h"
#include "io/datastream.h"

namespace MO {
namespace GEOM {

GeometryModifier::GeometryModifier(const QString &className)
    : className_    (className),
      enabled_      (true)
{

}

void GeometryModifier::serialize(IO::DataStream &io) const
{
    io.writeHeader("geommod", 1);

    io << enabled_;
}

void GeometryModifier::deserialize(IO::DataStream &io)
{
    io.readHeader("geommod", 1);

    io >> enabled_;
}

} // namespace GEOM
} // namespace MO
