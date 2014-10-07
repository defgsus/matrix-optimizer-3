/** @file polygon.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.10.2014</p>
*/

#include <QPolygonF>

#include "polygon.h"
#include "intersection.h"

namespace MO {
namespace MATH {


void limit(QVector<Vec2>& poly, const Vec2& minimum, const Vec2& maximum)
{
    for (auto & p : poly)
        p = glm::clamp(p, minimum, maximum);
}



QVector<Vec2> get_intersection_poly_rect(const QVector<Vec2> &poly, const Vec2 &rect_br, const Vec2 &rect_tl)
{

#define MO__INSIDE(vec__) \
    (inside_range(vec__, rect_br, rect_tl))

    // find a point which is outside
    int out = -1;
    for (int i; i<poly.size(); ++i)
    {
        if (!MO__INSIDE(poly[i]))
        {
            out = i;
            break;
        }
    }

#undef MO_INSIDE

    // no point is outside
    // intersection is congruent with poly
    if (out < 0)
        return poly;

    QPolygonF qrect, qpoly;
    qrect << QPointF(rect_br[0], rect_br[1])
          << QPointF(rect_tl[0], rect_br[1])
          << QPointF(rect_tl[0], rect_tl[1])
          << QPointF(rect_br[0], rect_tl[1]);

    for (auto & p : poly)
        qpoly << QPointF(p[0], p[1]);

    qpoly = qrect.intersected(qpoly);

    QVector<Vec2> ret;
    for (auto & p : qpoly)
        ret.append(Vec2(p.x(), p.y()));

    return ret;
}


} // namespace MATH
} // namespace MO
