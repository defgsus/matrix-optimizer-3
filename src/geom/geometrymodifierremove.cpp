/** @file geometrymodifierremove.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifierremove.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierRemove)

GeometryModifierRemove::GeometryModifierRemove()
    : GeometryModifier("Remove", QObject::tr("remove primitives")),
      prob_     (0.1),
      seed_     (0)
{

}

void GeometryModifierRemove::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("georemove", 1);

    io << prob_ << seed_;
}

void GeometryModifierRemove::deserialize(IO::DataStream &io)
{
    io.readHeader("georemove", 1);

    io >> prob_ >> seed_;
}

void GeometryModifierRemove::execute(Geometry *g)
{
    g->removePrimitivesRandomly(prob_, seed_);
}

} // namespace GEOM
} // namespace MO
