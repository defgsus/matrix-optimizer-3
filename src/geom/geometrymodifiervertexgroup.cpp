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

}

void GeometryModifierVertexGroup::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geovshare", 1);

    io << share_ << threshold_;
}

void GeometryModifierVertexGroup::deserialize(IO::DataStream &io)
{
    io.readHeader("geovshare", 1);

    io >> share_ >> threshold_;
}

void GeometryModifierVertexGroup::execute(Geometry *g)
{
    if (share_ && (!g->sharedVertices() || g->sharedVerticesThreshold() != threshold_))
    {
        Geometry geo(*g);
        g->clear();
        g->setSharedVertices(true, threshold_);
        g->copyFrom(geo);
    }

    if (!share_)
    {
        g->setSharedVertices(false);
        g->unGroupVertices();
    }
}

} // namespace GEOM
} // namespace MO
