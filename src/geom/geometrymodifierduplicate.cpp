/** @file geometrymodifierduplicate.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.10.2014</p>
*/

#include "geometrymodifierduplicate.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierDuplicate)

GeometryModifierDuplicate::GeometryModifierDuplicate()
    : GeometryModifier("Duplicate", QObject::tr("primitive equation")),
      equX_     ("4 * dx + x"),
      equY_     ("4 * dy + y"),
      equZ_     ("4 * dz + z"),
      numX_     (2),
      numY_     (1),
      numZ_     (1)
{

}

QString GeometryModifierDuplicate::statusTip() const
{
    return QObject::tr("Duplicates the geometry along "
                       "three axis and applies an equation to each coordinate "
                       "of the vertices of each new primitive");
}


void GeometryModifierDuplicate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geoequdup", 1);

    io << numX_ << numY_ << numZ_ << equX_ << equY_ << equZ_;
}

void GeometryModifierDuplicate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geoequdup", 1);

    io >> numX_ >> numY_ >> numZ_ >> equX_ >> equY_ >> equZ_;
}

void GeometryModifierDuplicate::execute(Geometry *g)
{
    Geometry copy(*g);

    QStringList constants =
    {
        "d", "dx", "dy", "dz"
    };

    for (uint z=0; z<numZ_; ++z)
    for (uint y=0; y<numY_; ++y)
    for (uint x=0; x<numX_; ++x)
    {
        Geometry geom(copy);
        geom.transformWithEquation(
                    equX_, equY_, equZ_,
                    constants,
                    QList<Double>()
                        << ((z*numY_+y)*numX_) << x << y << z
                    );

        g->addGeometry(geom);
    }
}

} // namespace GEOM
} // namespace MO
