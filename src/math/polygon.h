/** @file polygon.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 07.10.2014</p>
*/

#ifndef MOSRC_MATH_POLYGON_H
#define MOSRC_MATH_POLYGON_H

#include <QVector>

#include "types/vector.h"

namespace MO {
namespace MATH {


/** Limits all values in @p poly to the range [minimum, maximum] */
void limit(QVector<Vec2>& poly, const Vec2& minimum, const Vec2& maximum);

/** Returns the intersection of a polygon and a rectangle.
    Returns an empty list if no intersection occures. */
QVector<Vec2> get_intersection_poly_rect(const QVector<Vec2>& poly, const Vec2& rect_br, const Vec2& rect_tl);


} // namespace MATH
} // namespace MO


#endif // POLYGON_H

