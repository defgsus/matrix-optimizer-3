/** @file geometrymodifierrotate.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "GeometryModifierRotate.h"
#include "io/DataStream.h"
#include "Geometry.h"
#include "math/vector.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierRotate)

GeometryModifierRotate::GeometryModifierRotate()
    : GeometryModifier("Rotate", QObject::tr("rotate"))
{
    properties().set(
        "angle", QObject::tr("angle (degree)"),
        QObject::tr("The rotation angle around the given axis in degree"),
        0.f);
    properties().set(
        "x", QObject::tr("x-axis"),
        QObject::tr("The x component of the axis vector (internally normalized)"),
        1.f);
    properties().set(
        "y", QObject::tr("y-axis"),
        QObject::tr("The y component of the axis vector (internally normalized)"),
        0.f);
    properties().set(
        "z", QObject::tr("z-axis"),
        QObject::tr("The z component of the axis vector (internally normalized)"),
        0.f);
}

QString GeometryModifierRotate::statusTip() const
{
    return QObject::tr("Rotates the vertices (normals are *currently* left untouched)");
}


void GeometryModifierRotate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("georotate", 2);
}

void GeometryModifierRotate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const int ver = io.readHeader("georotate", 2);

    if (ver < 2)
    {
        Float angle, x, y, z;
        io >> angle >> x >> y >> z;
        properties().set("angle", angle);
        properties().set("x", x);
        properties().set("y", y);
        properties().set("z", z);
    }
}

void GeometryModifierRotate::execute(Geometry *g)
{
    const Mat4 rot = MATH::rotate(Mat4(1.0),
                properties().get("angle").toFloat(),
                        Vec3(
                            properties().get("x").toFloat(),
                            properties().get("y").toFloat(),
                            properties().get("z").toFloat()
                            )
                                  );
    g->applyMatrix(rot);
}

} // namespace GEOM
} // namespace MO
