/** @file geometrymodifier.cpp

    @brief Abstract base of Geometry modifiers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "GeometryModifier.h"
#include "GeometryModifierChain.h"
#include "types/Properties.h"
#include "io/DataStream.h"

namespace MO {
namespace GEOM {

bool registerGeometryModifier_(GeometryModifier * g)
{
    return GeometryModifierChain::registerModifier(g);
}

GeometryModifier::GeometryModifier(const QString &className, const QString &guiName)
    : p_className_  (className),
      p_guiName_    (guiName),
      p_enabled_    (true),
      p_progress_   (0),
      p_curObject_  (0)
{

}

bool GeometryModifier::operator != (const GeometryModifier& o) const
{
    return p_enabled_ != o.p_enabled_
           || properties() != o.properties();
}

void GeometryModifier::serialize(IO::DataStream &io) const
{
    io.writeHeader("geommod", 2);

    io << p_enabled_;

    // v2
    properties().serialize(io);
}

void GeometryModifier::deserialize(IO::DataStream &io)
{
    const int ver = io.readHeader("geommod", 2);

    io >> p_enabled_;

    if (ver >= 2)
        properties().deserialize(io);
}


void GeometryModifier::executeBase(Geometry *g, MO::Object * o)
{
    p_curObject_ = o;
    p_progress_ = 0.;
    execute(g);
}

} // namespace GEOM
} // namespace MO
