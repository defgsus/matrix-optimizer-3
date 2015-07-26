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
    properties().set(
        "prob", QObject::tr("probability"),
        QObject::tr("Probability of removing a primitive, between 0 and 1"),
        0.1, 0.0, 1.0, 0.025);
    properties().set(
        "seed", QObject::tr("random seed"),
        QObject::tr("Random seed which determines the pattern of removal"),
        int(1));
}

QString GeometryModifierRemove::statusTip() const
{
    return QObject::tr("Randomly removes primitives (lines or triangles)");
}

void GeometryModifierRemove::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("georemove", 1);

    io << prob_ << seed_;
}

void GeometryModifierRemove::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("georemove", 1);

    Float prob;
    int seed;
    io >> prob >> seed;
    properties().set("prob", prob);
    properties().set("seed", seed);
}

void GeometryModifierRemove::execute(Geometry *g)
{
    g->removePrimitivesRandomly(
                properties().get("prob").toFloat(),
                properties().get("seed").toInt()
                );
}

} // namespace GEOM
} // namespace MO
