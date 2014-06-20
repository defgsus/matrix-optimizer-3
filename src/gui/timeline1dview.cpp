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
        handleRadiusHovered_(4),
        hoverHash_  (Timeline1D::InvalidHash),
        action_     (A_NOTHING)
{
    setMinimumSize(100,60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

void Timeline1DView::setTimeline(Timeline1D *timeline)
{
    tl_ = timeline;

    update();
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

QRect Timeline1DView::handleRect_(const Timeline1D::Point& p, bool hovered)
{
    if (!hovered)
    {
        QRect r(0,0,handleRadius_*2, handleRadius_*2);
        r.moveTo(time2screen(p.t) - handleRadius_,
                 value2screen(p.val) - handleRadius_);
        return r;
    }

    QRect r(0,0,handleRadiusHovered_*2, handleRadiusHovered_*2);
    r.moveTo(time2screen(p.t) - handleRadiusHovered_,
             value2screen(p.val) - handleRadiusHovered_);
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
              i1 = e->rect().right() + 1,
              im = (i1 - i0) * overPaint_;

    p.setPen(QPen(QColor(0,255,0)));
    p.setBrush(Qt::NoBrush);

    int y0=0, x0=0, y;
    for (int i=0; i<=im; ++i)
    {
        Double x = i0 + (Double)i / overPaint_;
        y = value2screen( tl_->get(screen2time(x)) );
        if (i!=0)
            p.drawLine(x0, y0, x, y);
        x0 = x;
        y0 = y;
    }

    // -- handles --

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(QColor(255,255,0,200)));

    auto it0 = tl_->first(screen2time(i0)),
         it1 = tl_->first(screen2time(i1));

    for (auto it = it0; it != it1 && it != tl_->getData().end(); ++it)
    {
        const bool hovered = (hoverHash_ == it->first);
        const bool selected = (selectHashSet_.contains(it->first));

        if (selected)
            p.setBrush(QBrush(QColor(200,255,200,200)));
        else
        if (hovered)
            p.setBrush(QBrush(QColor(250,250,150,200)));
        else
            p.setBrush(QBrush(QColor(200,200,100,200)));

        p.drawRect(handleRect_(it->second, hovered));
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
            update(handleRect_(it1->second, true));
    }
}

void Timeline1DView::setHover_(const Timeline1D::Point & p)
{
    auto hash = Timeline1D::hash(p.t);

    bool newstate = hash != hoverHash_;

    hoverHash_ = hash;
    update(handleRect_(p, true));

    if (newstate && tl_)
    {
        // remove old flag
        auto it1 = tl_->getData().lower_bound(hoverHash_);
        if (it1 != tl_->getData().end())
            update(handleRect_(it1->second, true));
    }

}

bool Timeline1DView::isHover_() const
{
    return hoverHash_ != Timeline1D::InvalidHash;
}

Timeline1D::TpList::iterator Timeline1DView::hoverPoint_()
{
    if (!tl_)
        return Timeline1D::TpList::iterator();

    return tl_->getData().lower_bound(hoverHash_);
}

void Timeline1DView::clearSelect_()
{
    if (!isSelected_())
        return;

    // remove old flags
    for (auto &i : selectHashSet_)
    {
        auto it1 = tl_->getData().lower_bound(i);
        if (it1 != tl_->getData().end())
            update(handleRect_(it1->second, true));
    }
    selectHashSet_.clear();
}

void Timeline1DView::addSelect_(const Timeline1D::Point & p)
{
    auto hash = Timeline1D::hash(p.t);

    if (selectHashSet_.contains(hash))
        return;

    selectHashSet_.insert(hash);

    update(handleRect_(p, true));
}


void Timeline1DView::mouseMoveEvent(QMouseEvent * e)
{
    if (!tl_)
        return;

    // mouse coords in timeline space
    Double x = screen2time(e->x()),
           y = screen2value(e->y()),
    // handle radius in timeline space
           rx = screen2time(e->x()+handleRadius_) - x,
           ry = screen2value(e->y()-handleRadius_) - y;

    if (action_ == A_START_DRAG_SELECTED)
    {
        // mouse delta in timeline space
        Double dx = screen2time(e->x()) - screen2time(dragStart_.x()),
               dy = screen2value(e->y()) - screen2value(dragStart_.y());

        moveSelected_(dx, dy);
    }



    // --------- on hover over --------------

    auto oldHoverHash = hoverHash_;

    // get the timeline points in range
    auto it = tl_->first(x-rx),
         last = tl_->next_after(x+rx);
    // iterate over all of them
    for (; it != tl_->getData().end() && it != last; ++it)
    {
        if (x >= it->second.t - rx
         && x <= it->second.t + rx
         && y >= it->second.val - ry
         && y <= it->second.val + ry)
        {
            if (it->first == oldHoverHash)
                return;

            // set new hover point
            setHover_(it->second);

            e->accept();
            return;
        }
    }
    // unhover
    clearHover_();

}

void Timeline1DView::mousePressEvent(QMouseEvent * e)
{
    if (!tl_)
        return;

    if (isHover_())
    {
        if (e->button() == Qt::LeftButton)
        {
            auto point = hoverPoint_();
            // fail-safe (should be a point though)
            if (point == tl_->getData().end())
                return;

            // keep selection when shift pressed
            if (!(e->modifiers() & Qt::SHIFT))
                clearSelect_();

            addSelect_(point->second);
            action_ = A_START_DRAG_SELECTED;
            dragStart_ = e->pos();
            e->accept();
            // copy the points to drag
            dragPoints_.clear();
            for (auto &h : selectHashSet_)
            {
                auto it = tl_->getData().lower_bound(h);
                if (it != tl_->getData().end())
                    dragPoints_.push_back(DragPoint_(it->second));
            }
            return;
        }
    }
}

void Timeline1DView::mouseReleaseEvent(QMouseEvent * )
{
    action_ = A_NOTHING;
}


void Timeline1DView::moveSelected_(Double dx, Double dy)
{
    if (!tl_)
        return;

    // only move vertically?
    if (dx == 0)
    {
        for (auto &p : dragPoints_)
        {
            auto it = tl_->first(p.newp.t);

            if (it == tl_->getData().end())
                continue;

            it->second.val = p.oldp.val + dy;
        }

        update();
        return;
    }

    for (auto &p : dragPoints_)
    {
        auto it = tl_->first(p.newp.t);

        if (it == tl_->getData().end())
            continue;

        // delete previous point
        tl_->getData().erase(it);

        // create a new one
        auto newp = tl_->add(p.oldp.t + dx, p.oldp.val + dy, p.oldp.type);

        if (newp != 0)
            p.newp = *newp;
    }

    update();
}

} // namespace GUI
} // namespace MO
