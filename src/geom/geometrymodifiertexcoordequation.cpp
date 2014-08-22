/** @file geometrymodifiertexcoordequation.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#include "geometrymodifiertexcoordequation.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierTexCoordEquation)

GeometryModifierTexCoordEquation::GeometryModifierTexCoordEquation()
    : GeometryModifier("TexCoordEquation", QObject::tr("texture coord. equation")),
      equS_     ("s"),
      equT_     ("t")
{

}

void GeometryModifierTexCoordEquation::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoequtex", 1);

    io << equS_ << equT_;
}

void GeometryModifierTexCoordEquation::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geoequtex", 1);

    io >> equS_ >> equT_;
}

void GeometryModifierTexCoordEquation::execute(Geometry *g)
{
    g->transformTexCoordsWithEquation(equS_, equT_);
}

} // namespace GEOM
} // namespace MO
