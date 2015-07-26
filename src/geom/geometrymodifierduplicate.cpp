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
      equ_      ("x = x + 2 * dx;\n"
                 "y = y + 2 * dy"),
      numX_     (2),
      numY_     (2),
      numZ_     (1)
{
    properties().set(
        "equ", QObject::tr("equation"),
        QObject::tr("Equation to modify the vertex coordinates"),
        QString(
            "x = x + 2 * dx;\n"
            "y = y + 2 * dy;\n"
            "z = z + 2 * dz;")
        );

    properties().set(
        "numx", QObject::tr("number x"),
        QObject::tr("Number of duplications on x-axis"),
        2u);
    properties().setMin("numx", 1u);

    properties().set(
        "numy", QObject::tr("number y"),
        QObject::tr("Number of duplications on y-axis"),
        2u);
    properties().setMin("numy", 1u);

    properties().set(
        "numz", QObject::tr("number z"),
        QObject::tr("Number of duplications on z-axis"),
        2u);
    properties().setMin("numz", 1u);
}

QString GeometryModifierDuplicate::statusTip() const
{
    return QObject::tr("Duplicates the geometry along "
                       "three axis and applies an equation to the vertex "
                       "coordiantes of each new primitive");
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

    uint numX, numY, numZ;
    io >> numX >> numY >> numZ;

    QString equ;
    if (ver<2)
    {
        QString equx, equy, equz;
        io >> equx >> equy >> equz;
        equ = "x = " + equx + ";\ny = " + equy + ";\nz = " + equz;
    }
    else
        io >> equ;

    properties().set("equ", equ);
    properties().set("numX", numX);
    properties().set("numY", numY);
    properties().set("numZ", numZ);
}

void GeometryModifierDuplicate::execute(Geometry *g)
{
    Geometry *copy = new Geometry(*g);

    g->clear();

    QStringList constants =
    {
        "d", "dx", "dy", "dz"
    };

    const uint
            numX = properties().get("numX").toUInt(),
            numY = properties().get("numY").toUInt(),
            numZ = properties().get("numZ").toUInt();
    const QString
            equ = properties().get("equ").toString();

    for (uint z=0; z<numZ; ++z)
    for (uint y=0; y<numY; ++y)
    for (uint x=0; x<numX; ++x)
    {
        Geometry * geom = new Geometry(*copy);
        geom->transformWithEquation(
                    equ,
                    constants,
                    QList<Double>()
                        << ((z*numY+y)*numX+x) << x << y << z
                    );

        g->addGeometry(*geom);

        geom->releaseRef();
    }

    copy->releaseRef();
}

} // namespace GEOM
} // namespace MO
