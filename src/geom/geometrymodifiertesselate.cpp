/** @file geometrymodifiertesselate.cpp

    @brief Tesselates a Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifiertesselate.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierTesselate)

GeometryModifierTesselate::GeometryModifierTesselate()
    : GeometryModifier("Tesselate", QObject::tr("tesselate")),
      level_    (1)
    , minArea_  (0.)
    , minLength_(0.)
{

}

QString GeometryModifierTesselate::statusTip() const
{
    return QObject::tr("Sub-triangulates triangles to a given level - "
                       "be careful, size of model grows exponetially!");
}


void GeometryModifierTesselate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotess", 2);

    io << level_;
    // v2
    io << minArea_ << minLength_;
}

void GeometryModifierTesselate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    int ver = io.readHeader("geotess", 2);

    io >> level_;
    if (ver >= 2)
        io >> minArea_ >> minLength_;
}


void GeometryModifierTesselate::execute(Geometry *g)
{
    g->tesselateTriangles(minArea_, minLength_, level_);
}


} // namespace GEOM
} // namespace MO
