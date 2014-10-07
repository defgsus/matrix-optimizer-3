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
    : GeometryModifier("Duplicate", QObject::tr("duplicate")),
      equ_      ("x = 2 * dx + x;\n"
                 "y = 2 * dy + y;\n"),
      numX_     (2),
      numY_     (2),
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

    io.writeHeader("geoequdup", 2);

    io << numX_ << numY_ << numZ_ << equ_;
}

void GeometryModifierDuplicate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("geoequdup", 2);

    io >> numX_ >> numY_ >> numZ_;

    if (ver<2)
    {
        QString equx, equy, equz;
        io >> equx >> equy >> equz;
        equ_ = "x = " + equx + ";\ny = " + equy + ";\nz = " + equz;
    }
    else
        io >> equ_;
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
                    equ_,
                    constants,
                    QList<Double>()
                        << ((z*numY_+y)*numX_+x) << x << y << z
                    );

        g->addGeometry(geom);
    }
}

} // namespace GEOM
} // namespace MO
