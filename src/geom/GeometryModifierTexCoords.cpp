/** @file geometrymodifiertexcoords.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#include "GeometryModifierTexCoords.h"
#include "io/DataStream.h"
#include "Geometry.h"

namespace MO {
namespace GEOM {

MO_REGISTER_GEOMETRYMODIFIER(GeometryModifierTexCoords)

GeometryModifierTexCoords::GeometryModifierTexCoords()
    : GeometryModifier("TexCoords", QObject::tr("texture coordinates"))
{
    properties().set(
        "doMapTri", QObject::tr("map triangles"),
        QObject::tr("Assigns the texture coordinates to the triangle corners"),
        false);

    properties().set(
        "offsetX", QObject::tr("x offset"),
        QObject::tr("Offset added to the x-axis"),
        0.f, 0.1f);
    properties().set(
        "offsetY", QObject::tr("y offset"),
        QObject::tr("Offset added to the y-axis"),
        0.f, 0.1f);
    properties().set(
        "scaleX", QObject::tr("x scale"),
        QObject::tr("Scale on the x-axis"),
        1.f, 0.1f);
    properties().set(
        "scaleY", QObject::tr("y scale"),
        QObject::tr("Scale on the y-axis"),
        1.f, 0.1f);

    properties().set(
        "invertX", QObject::tr("invert x coordinates"),
        QObject::tr("Inverts the texture coordinates on the x-axis"),
        false);
    properties().set(
        "invertY", QObject::tr("invert y coordinates"),
        QObject::tr("Inverts the texture coordinates on the y-axis"),
        false);
}

QString GeometryModifierTexCoords::statusTip() const
{
    return QObject::tr("Modifies existing texture coordinates (offset, scale, invert)");
}

void GeometryModifierTexCoords::serialize(IO::DataStream &io) const
{
    GeometryModifier::serialize(io);

    io.writeHeader("geotexcoords", 3);
}

void GeometryModifierTexCoords::deserialize(IO::DataStream &io)
{
    GeometryModifier::deserialize(io);

    const auto ver = io.readHeader("geotexcoords", 3);

    if (ver < 3)
    {
        Float offsetX, offsetY,
              scaleX, scaleY;
        bool invertX, invertY, doMapTri=false;

        io >> offsetX >> offsetY
           >> scaleX >> scaleY
           >> invertX >> invertY;

        if (ver >= 2)
            io >> doMapTri;

        properties().set("offsetX", offsetX);
        properties().set("offsetY", offsetY);
        properties().set("scaleX", scaleX);
        properties().set("scaleY", scaleY);
        properties().set("invertX", invertX);
        properties().set("invertY", invertY);
        properties().set("doMapTri", doMapTri);
    }
}

void GeometryModifierTexCoords::execute(Geometry *g)
{
    // map triangles
    if (properties().get("doMapTri").toBool())
    {
        for (uint i=0; i<g->numTriangles(); ++i)
        {
            g->setTexCoord(g->triangleIndex(i, 0), Vec2(0,0));
            g->setTexCoord(g->triangleIndex(i, 1), Vec2(0,1));
            g->setTexCoord(g->triangleIndex(i, 2), Vec2(1,1));
        }
    }

    const Float
            offsetX = properties().get("offsetX").toFloat(),
            offsetY = properties().get("offsetY").toFloat(),
            scaleX = properties().get("scaleX").toFloat(),
            scaleY = properties().get("scaleY").toFloat();
    const bool
            invertX = properties().get("invertX").toBool(),
            invertY = properties().get("invertY").toBool();

    if (offsetX != 0.f || offsetY != 0.f)
        g->shiftTextureCoords(offsetX, offsetY);

    if (scaleX != 1.f || scaleY != 1.f)
        g->scaleTextureCoords(scaleX, scaleY);

    if (invertX || invertY)
        g->invertTextureCoords(invertX, invertY);
}

} // namespace GEOM
} // namespace MO
