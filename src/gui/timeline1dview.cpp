/** @file

    @brief widget for Timeline1D

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/24</p>
*/

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>

#include "timeline1dview.h"

namespace MO {
namespace GUI {

Timeline1DView::Timeline1DView(Timeline1D * tl, QWidget *parent)
    :   QWidget     (parent),
        tl_         (tl),
        overPaint_  (4),
        handleRadius_(3),
        hoverHash_   (Timeline1D::InvalidHash)
{
    setMinimumSize(100,60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void Timeline1DView::setTimeline(Timeline1D *timeline)
{
    tl_ = timeline;

    repaint();
}

Double Timeline1DView::screen2time(Double x) const
{
    return x / width() * 10.0;
}

Double Timeline1DView::screen2value(Double y) const
{
    return (height() - 1 - y) / height();
}

int Timeline1DView::time2screen(Double time) const
{
    return time / 10.0 * width();
}

int Timeline1DView::value2screen(Double val) const
{
    return height() - 1 - val * height();
}

QRect Timeline1DView::handleRect_(const Timeline1D::Point& p)
{
    QRect r(0,0,handleRadius_*2, handleRadius_*2);
    r.moveTo(time2screen(p.t) - handleRadius_,
             value2screen(p.val) - handleRadius_);
    return r;
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
        y = value2screen( tl_->get(screen2time(x)) );
        if (x!=0)
            p.drawLine(x-1, y0, x, y);
        y0 = y;
    }

    // -- handles --

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(QColor(255,255,0,200)));

    auto it0 = tl_->first(screen2time(i0)),
         it1 = tl_->first(screen2time(i1));

    for (auto it = it0; it != it1 && it != tl_->getData().end(); ++it)
    {
        if (hoverHash_ == it->first)
            p.setBrush(QBrush(QColor(255,255,255,200)));
        else
            p.setBrush(QBrush(QColor(255,255,0,200)));

        p.drawRect(handleRect_(it->second));
    }
}

void Timeline1DView::clearHover_()
{
    auto wasHash = hoverHash_;
    hoverHash_ = Timeline1D::InvalidHash;

    // remove old flag
    if (wasHash != hoverHash_)
    {
        auto it1 = tl_->getData().lower_bound(wasHash);
        if (it1 != tl_->getData().end())
            repaint(handleRect_(it1->second));
    }
}

void Timeline1DView::setHover_(const Timeline1D::Point & p)
{
    auto hash = Timeline1D::hash(p.t);

    bool newstate = hash != hoverHash_;

    hoverHash_ = hash;
    repaint(handleRect_(p));

    if (newstate)
    {
        // remove old flag
        auto it1 = tl_->getData().lower_bound(hoverHash_);
        if (it1 != tl_->getData().end())
            repaint(handleRect_(it1->second));
    }

}

void Timeline1DView::mouseMoveEvent(QMouseEvent * e)
{
    if (!tl_)
        return;

    Double x = screen2time(e->x()),
           y = screen2value(e->y()),
           // handle radius in timeline space
           rx = screen2time(e->x()+handleRadius_) - x,
           ry = screen2value(e->y()-handleRadius_) - y;

    auto oldHoverHash = hoverHash_;

    auto it = tl_->closest(x);
    if (it != tl_->getData().end())
    {
        if (x >= it->second.t - rx
         && x <= it->second.t + rx
         && y >= it->second.val - ry
         && y <= it->second.val + ry)
        {
            if (it->first == oldHoverHash)
                return;
            setHover_(it->second);

            e->accept();
            return;
        }
    }
    // unhover
    clearHover_();

}

void Timeline1DView::mousePressEvent(QMouseEvent * )
{

}

void Timeline1DView::mouseReleaseEvent(QMouseEvent * )
{

}


} // namespace GUI
} // namespace MO
