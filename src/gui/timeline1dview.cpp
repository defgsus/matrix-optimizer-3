/** @file

    @brief widget for Timeline1D

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/24</p>
*/

#include <QPaintEvent>
#include <QPainter>

#include "timeline1dview.h"
#include "math/timeline1d.h"

namespace MO {
namespace GUI {

Timeline1DView::Timeline1DView(Timeline1D * tl, QWidget *parent) :
    QWidget(parent),
    tl_ (tl)
{
    setMinimumSize(100,60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void Timeline1DView::setTimeline(Timeline1D *timeline)
{
    tl_ = timeline;

    repaint(rect());
}

Double Timeline1DView::screen2time(int x) const
{
    return (Double)x / width() * 10.0;
}

void Timeline1DView::paintEvent(QPaintEvent * e)
{
    QPainter p(this);

    // -- background --
    p.setPen(Qt::NoPen);
    //p.setBrush(QBrush(QColor(palette().color(backgroundRole()))));
    p.setBrush(QBrush(QColor(50,50,50)));
    p.drawRect(e->rect());

    if (!tl_)
        return;

    // -- curve --

    const int i0 = e->rect().left(),
              i1 = e->rect().right();

    p.setPen(QPen(QColor(0,255,0)));
    p.setBrush(Qt::NoBrush);

    int y0=0, y;
    for (int i=i0; i<=i1; ++i)
    {
        y = height() - 1 - tl_->get(screen2time(i)) * height();
        if (i!=i0)
            p.drawLine(i-1, y0, i, y);
        y0 = y;
    }
}



} // namespace GUI
} // namespace MO
