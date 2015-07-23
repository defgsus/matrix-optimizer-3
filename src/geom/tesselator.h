/** @file tesselator.h

    @brief QPolygon to triangle tesselator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.09.2014</p>
*/

#ifndef MOSRC_GEOM_TESSELATOR_H
#define MOSRC_GEOM_TESSELATOR_H

#include <QVector>
#include <QPointF>

#include "types/int.h"
#include "types/vector.h"

namespace MO {
namespace GEOM {

class TesselatorPrivate;
class Geometry;

class Tesselator
{
public:
    Tesselator();
    ~Tesselator();

    void clear();

    // -------------- tesselation ------------------

    /** Tesselates the polygon described by @p points.
        The points must describe the contour of a polygon in anti-clockwise order.
        If @p trianglesOnly is true, no GL_TRIANGLE_FAN or GL_TRIANGLE_STRIP will be created.
        Any previous data is cleared. */
    void tesselate(const QVector<Vec2> & points, bool trianglesOnly = false);

    /** Tesselates the polygon described by @p points.
        The points must describe the contour of a polygon in anti-clockwise order.
        If @p trianglesOnly is true, no GL_TRIANGLE_FAN or GL_TRIANGLE_STRIP will be created.
        Any previous data is cleared. */
    void tesselate(const QVector<DVec2> & points, bool trianglesOnly = false);

    /** Tesselates the polygon described by @p points.
        The points must describe the contour of a polygon in anti-clockwise order.
        If @p trianglesOnly is true, no GL_TRIANGLE_FAN or GL_TRIANGLE_STRIP will be created.
        Any previous data is cleared. */
    void tesselate(const QVector<QPointF> & points, bool trianglesOnly = false);

    // -------------- getter -----------------------

    /** Returns true if tesselation was successful */
    bool isValid() const;

    /** Returns the number of (unshared) vertices after tesselation */
    uint numVertices() const;
    /** Returns the number of triangles after tesselation */
    uint numTriangles() const;

    /** Adds the tesselated triangles to the Geometry object.
        If tesselation was unsuccessful, nothing is added. */
    void getGeometry(Geometry&, bool asTriangles = true) const;

    /** Creates an Geometry object with the tesselated triangles.
        If tesselation was unsuccessful, an empty Geometry is returned. */
    Geometry * getGeometry(bool asTriangles = true) const;

private:

    TesselatorPrivate * p_;

};

} // namespace GEOM
} // namespace MO

#endif // TESSELATOR_H
