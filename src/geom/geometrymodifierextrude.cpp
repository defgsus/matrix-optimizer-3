/** @file geometrymodifierextrude.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifierextrude.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierExtrude)

GeometryModifierExtrude::GeometryModifierExtrude()
    : GeometryModifier("Extrude", QObject::tr("Extrude")),
      constant_ (0.1),
      factor_   (0.0)
{

}

void GeometryModifierExtrude::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoextrude", 1);

    io << constant_ << factor_;
}

void GeometryModifierExtrude::deserialize(IO::DataStream &io)
{
    io.readHeader("geoextrude", 1);

    io >> constant_ >> factor_;
}

void GeometryModifierExtrude::execute(Geometry *g)
{
    if (!g->numTriangles())
        return;

    Geometry geom;
    geom.setSharedVertices(g->sharedVertices(), g->sharedVerticesThreshold());
    g->extrudeTriangles(geom, constant_, factor_);
    if (!g->sharedVertices())
        geom.unGroupVertices();
    *g = geom;
}

} // namespace GEOM
} // namespace MO
