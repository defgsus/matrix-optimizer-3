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

namespace MO {
namespace GEOM {

class TesselatorPrivate;
class Geometry;

class Tesselator
{
public:
    Tesselator();
    ~Tesselator();

    // -------------- tesselation ------------------

    /** Tesselates the polygon described by @p points.
        The points must describe the contour of a polygon in anti-clockwise order.
        Any previous data is cleared. */
    void tesselate(const QVector<QPointF> & points);

    // -------------- getter -----------------------

    /** Returns true if tesselation was successful */
    bool isValid() const;

    /** Returns the number of (unshared) vertices after tesselation */
    uint numVertices() const;
    /** Returns the number of triangles after tesselation */
    uint numTriangles() const;

    /** Adds the tesselated triangles to the Geometry object.
        If tesselation was unsuccessful, nothing is added. */
    void getGeometry(Geometry&) const;

    /** Creates an Geometry object with the tesselated triangles.
        If tesselation was unsuccessful, an empty Geometry is returned. */
    Geometry * getGeometry() const;

private:

    TesselatorPrivate * p_;

};

} // namespace GEOM
} // namespace MO

#endif // TESSELATOR_H
