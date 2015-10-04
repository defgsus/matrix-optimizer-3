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

/** Class for tesselating polygons.
    Internally uses GLUT at the moment.
    Default input is QVector<DVec2>, output is a GEOM::Geometry
*/
class Tesselator
{
public:
    Tesselator();
    ~Tesselator();

    /** Forgets all previously passed data */
    void clear();

    /** Set a basis mesh for triangulation. Only the area of the polygon,
        intersecting with the given @p triangles is tesselated.
        The resulting (tesselated) triangles will never be coarser than
        the given mesh. This may help with cases where big or long and thin
        triangles are created and you need a finer resolution for further
        processing. */
    void setTriangulationMesh(const QVector<DVec2>& triangles);

    /** Sets a basis mesh for triangulation. A quad mesh is created from
        @p minExt to @p maxExt with a stepsize of @p spacing.
        @see setTriangulationMesh() */
    void createTriangulationMesh(const DVec2& minExt,
                                 const DVec2& maxExt, const DVec2& spacing);

    /** Returns the min and max extends of the vectors in @p polygon. */
    static void getExtend(const QVector<DVec2>& polygon,
                          DVec2& minEx, DVec2& maxEx);

    // -------------- tesselation ------------------

    void addPolygon(const QVector<Vec2>& points);
    void addPolygon(const QVector<DVec2>& points);
    void addPolygon(const QVector<QPointF>& points);
    void addPolygon(const QPolygonF& points);

    void tesselate();

    /** @{ */
    /** Tesselates the polygon described by @p points.
        The points must describe the contour of a polygon in anti-clockwise order.
        If @p trianglesOnly is true, no GL_TRIANGLE_FAN or GL_TRIANGLE_STRIP will be created.
        Any previous data is cleared. */
    void tesselate(const QVector<Vec2> & points, bool trianglesOnly = false);
    void tesselate(const QVector<DVec2> & points, bool trianglesOnly = false);
    void tesselate(const QVector<QPointF> & points, bool trianglesOnly = false);
    /** @} */

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
