/** @file valuecurve.cpp

    @brief generic painter for f(x)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QPainter>
#include <QRect>
#include <QSize>

#include "valuecurve.h"

namespace MO {
namespace GUI {
namespace PAINTER {


ValueCurve::ValueCurve(QObject *parent) :
    QObject     (parent),
    data_       (0),
    pen_        (QColor(0,255,0)),
    overPaint_  (4)
{
}

void ValueCurve::setAlpha(int alpha)
{
    QColor c = pen_.color();
    c.setAlpha(alpha);
    pen_.setColor(c);
}



void ValueCurve::paint(QPainter & p)
{
    paint(p, p.window());
}

void ValueCurve::paint(QPainter &p, const QRect &rect)
{
    if (!data_)
        return;

    p.setPen(pen_);
    p.setBrush(Qt::NoBrush);

    overPaint_ = std::max(1, overPaint_);

    const int i0 = rect.left() - 1,
              i1 = rect.right() + 2,
              im = (i1 - i0) * overPaint_;

    int y0=0, x0=0, y;
    for (int i=0; i<=im; ++i)
    {
        Double x = i0 + (Double)i / overPaint_;
        y = p.window().height() - 1 - p.window().height() *
                viewspace_.mapYFrom( data_->value(viewspace_.mapXTo(x / p.window().width())) );

        if (i!=0)
            p.drawLine(x0, y0, x, y);

        x0 = x;
        y0 = y;
    }

}



} // namespace PAINTER
} // namespace GUI
} // namespace MO
