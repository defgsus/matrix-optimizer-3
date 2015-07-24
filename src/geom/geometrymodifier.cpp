/** @file geometrymodifier.cpp

    @brief Abstract base of Geometry modifiers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifier.h"
#include "geometrymodifierchain.h"
#include "types/properties.h"
#include "io/datastream.h"

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


void GeometryModifier::serialize(IO::DataStream &io) const
{
    io.writeHeader("geommod", 1);

    io << p_enabled_;
}

void GeometryModifier::deserialize(IO::DataStream &io)
{
    io.readHeader("geommod", 1);

    io >> p_enabled_;
}


void GeometryModifier::executeBase(Geometry *g, MO::Object * o)
{
    p_curObject_ = o;
    p_progress_ = 0.;
    execute(g);
}

} // namespace GEOM
} // namespace MO
