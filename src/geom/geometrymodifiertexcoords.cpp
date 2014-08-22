/** @file geometrymodifiertexcoords.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#include "geometrymodifiertexcoords.h"
#include "io/datastream.h"
#include "geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierTexCoords)

GeometryModifierTexCoords::GeometryModifierTexCoords()
    : GeometryModifier("TexCoords", QObject::tr("texture coordinates")),
      offsetX_  (0.0),
      offsetY_  (0.0),
      scaleX_   (1.0),
      scaleY_   (1.0),
      invertX_  (false),
      invertY_  (false)
{

}

QString GeometryModifierTexCoords::statusTip() const
{
    return QObject::tr("Modifies existing texture coordinates (offset, scale, invert)");
}

void GeometryModifierTexCoords::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotexcoords", 1);

    io << offsetX_ << offsetY_
       << scaleX_ << scaleY_
       << invertX_ << invertY_;
}

void GeometryModifierTexCoords::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    io.readHeader("geotexcoords", 1);

    io >> offsetX_ >> offsetY_
       >> scaleX_ >> scaleY_
       >> invertX_ >> invertY_;
}

void GeometryModifierTexCoords::execute(Geometry *g)
{
    if (offsetX_ != 0.0 || offsetY_ != 0.0)
        g->shiftTextureCoords(offsetX_, offsetY_);

    if (scaleX_ != 1.0 || scaleY_ != 1.0)
        g->scaleTextureCoords(scaleX_, scaleY_);

    if (invertX_ || invertY_)
        g->invertTextureCoords(invertX_, invertY_);
}

} // namespace GEOM
} // namespace MO
