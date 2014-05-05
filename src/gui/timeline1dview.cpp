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

Timeline1DView::Timeline1DView(Timeline1D * tl, QWidget *parent)
    :   QWidget     (parent),
        tl_         (tl),
        overPaint_  (4)
{
    setMinimumSize(100,60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void Timeline1DView::setTimeline(Timeline1D *timeline)
{
    tl_ = timeline;

    repaint(rect());
}

Double Timeline1DView::screen2time(Double x) const
{
    return x / width() * 10.0;
}

void Timeline1DView::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // -- background --
    p.setPen(Qt::NoPen);
    //p.setBrush(QBrush(QColor(palette().color(backgroundRole()))));
    p.setBrush(QBrush(QColor(50,50,50)));
    p.drawRect(e->rect());

    if (!tl_)
        return;

    // -- curve --

    overPaint_ = std::max(1, overPaint_);

    const int i0 = e->rect().left(),
              i1 = e->rect().right(),
              im = (i1 - i0) * overPaint_;

    p.setPen(QPen(QColor(0,255,0)));
    p.setBrush(Qt::NoBrush);

    int y0=0, y;
    for (int i=0; i<=im; ++i)
    {
        Double x = (Double)i / overPaint_;
        y = height() - 1 - tl_->get(screen2time(x)) * height();
        if (x!=0)
            p.drawLine(x-1, y0, x, y);
        y0 = y;
    }
}



} // namespace GUI
} // namespace MO
