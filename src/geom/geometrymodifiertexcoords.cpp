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
      invertY_  (false),
      doMapTri_ (false)
{

}

QString GeometryModifierTexCoords::statusTip() const
{
    return QObject::tr("Modifies existing texture coordinates (offset, scale, invert)");
}

void GeometryModifierTexCoords::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotexcoords", 2);

    io << offsetX_ << offsetY_
       << scaleX_ << scaleY_
       << invertX_ << invertY_;

    io << doMapTri_;
}

void GeometryModifierTexCoords::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const auto ver = io.readHeader("geotexcoords", 2);

    io >> offsetX_ >> offsetY_
       >> scaleX_ >> scaleY_
       >> invertX_ >> invertY_;

    if (ver >= 2)
        io >> doMapTri_;
}

void GeometryModifierTexCoords::execute(Geometry *g)
{
    // map triangles
    if (doMapTri_)
    {
        for (uint i=0; i<g->numTriangles(); ++i)
        {
            g->setTexCoord(g->triangleIndex(i, 0), Vec2(0,0));
            g->setTexCoord(g->triangleIndex(i, 1), Vec2(0,1));
            g->setTexCoord(g->triangleIndex(i, 2), Vec2(1,1));
        }
    }

    if (offsetX_ != 0.0 || offsetY_ != 0.0)
        g->shiftTextureCoords(offsetX_, offsetY_);

    if (scaleX_ != 1.0 || scaleY_ != 1.0)
        g->scaleTextureCoords(scaleX_, scaleY_);

    if (invertX_ || invertY_)
        g->invertTextureCoords(invertX_, invertY_);
}

} // namespace GEOM
} // namespace MO
