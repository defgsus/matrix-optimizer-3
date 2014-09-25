/** @file testtesselator.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.09.2014</p>
*/

#include "testtesselator.h"
#include "geom/tesselator.h"
#include "geom/geometry.h"
#include "io/log.h"

namespace MO {


TestTesselator::TestTesselator()
{
}


int TestTesselator::run()
{
    QVector<QPointF> points;
    /*
    points.append(QPointF(0,0));
    points.append(QPointF(50,0));
    points.append(QPointF(10,50));
    points.append(QPointF(50,100));
    points.append(QPointF(0,100));
    */

    /*  _____
     * |  _  |
     * |_| |_|
     */
    points.append(QPointF(0,0));
    points.append(QPointF(10,0));
    points.append(QPointF(10,10));
    points.append(QPointF(20,10));
    points.append(QPointF(20,0));
    points.append(QPointF(30,0));
    points.append(QPointF(30,20));
    points.append(QPointF(0,20));

    GEOM::Tesselator tess;

    tess.tesselate(points);

    std::cerr.flush();
    std::cout.flush();

    GEOM::Geometry * g = tess.getGeometry();
    for (uint i=0; i<g->numTriangles(); ++i)
    {
        const GEOM::Geometry::IndexType
                i1 = g->triangleIndices()[i*3],
                i2 = g->triangleIndices()[i*3+1],
                i3 = g->triangleIndices()[i*3+2];

        const Vec3
                p1 = g->getVertex(i1),
                p2 = g->getVertex(i2),
                p3 = g->getVertex(i3);

        MO_PRINT(i << "\t" << p1 << ", " << p2 << ", " << p3);
    }

    return 0;
}

} // namespace MO
