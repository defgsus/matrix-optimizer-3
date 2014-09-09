/** @file geometrymodifierrotate.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include "geometrymodifierrotate.h"
#include "io/datastream.h"
#include "geometry.h"
#include "math/vector.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierRotate)

GeometryModifierRotate::GeometryModifierRotate()
    : GeometryModifier("Rotate", QObject::tr("rotate")),
      angle_    (0.0),
      x_        (1.0),
      y_        (0.0),
      z_        (0.0)
{

}

QString GeometryModifierRotate::statusTip() const
{
    return QObject::tr("Rotates the vertices (normals are left untouched)");
}


void GeometryModifierRotate::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("georotate", 1);

    io << angle_ << x_ << y_ << z_;
}

void GeometryModifierRotate::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("georotate", 1);

    io >> angle_ >> x_ >> y_ >> z_;
}

void GeometryModifierRotate::execute(Geometry *g)
{
    const Mat4 rot = MATH::rotate(Mat4(1.0), angle_, Vec3(x_, y_, z_));
    g->applyMatrix(rot);
}

} // namespace GEOM
} // namespace MO
