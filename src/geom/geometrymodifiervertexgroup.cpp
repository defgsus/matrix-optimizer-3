/** @file geometrymodifiervertexgroup.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiervertexgroup.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierVertexGroup)

GeometryModifierVertexGroup::GeometryModifierVertexGroup()
    : GeometryModifier("VertexGroup", QObject::tr("vertex sharing")),
      share_        (true),
      threshold_    (Geometry::minimumThreshold)
{
    properties().set(
        "shared", QObject::tr("shared vertices"),
        QObject::tr("When enabled, primitives share their vertices, "
                    "otherwise primitives are forced to use unique vertices"),
        true);

    properties().set(
        "threshold", QObject::tr("threshold"),
        QObject::tr("Threshold distance for which vertices are considered the same"),
        Geometry::minimumThreshold,
        Geometry::minimumThreshold, 1000000.f,
        Geometry::minimumThreshold);
}

QString GeometryModifierVertexGroup::statusTip() const
{
    return QObject::tr("Groups or un-groups vertices - that is, "
                       "makes primitives use shared or unique vertices");
}


void GeometryModifierVertexGroup::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geovshare", 1);

    io << share_ << threshold_;
}

void GeometryModifierVertexGroup::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geovshare", 1);

    bool share;
    Float threshold;
    io >> share >> threshold;
    properties().set("shared", share);
    properties().set("threshold", threshold);
}

void GeometryModifierVertexGroup::execute(Geometry *g)
{
    const bool share = properties().get("share").toBool();

    if (share && (!g->sharedVertices() || g->sharedVerticesThreshold() != threshold_))
    {
        Geometry * geo = new Geometry(*g);
        g->clear();
        g->setSharedVertices(true, properties().get("threshold").toFloat());
        g->addGeometry(*geo);
        geo->releaseRef();
    }

    if (!share)
    {
        g->setSharedVertices(false);
        g->unGroupVertices();
    }
}

} // namespace GEOM
} // namespace MO
