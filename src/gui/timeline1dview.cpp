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
        handleRadiusSelected_(6),
        hoverHash_  (Timeline1D::InvalidHash),
        action_     (A_NOTHING)
{
    space_.scaleX = 10;
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
    return x / width() * space_.scaleX - space_.offsetX;
}

Double Timeline1DView::screen2value(Double y) const
{
    return (height() - 1 - y) / height() * space_.scaleY - space_.offsetY;
}

int Timeline1DView::time2screen(Double time) const
{
    return (time + space_.offsetX) / space_.scaleX * width();
}

int Timeline1DView::value2screen(Double val) const
{
    return height() - 1 - (val + space_.offsetY) * height() / space_.scaleY;
}

QRect Timeline1DView::handleRect_(const Timeline1D::Point& p, RectStyle_ rs)
{
    int r = handleRadius_;

    switch (rs)
    {
        case RS_HOVER: r = handleRadiusHovered_; break;
        case RS_SELECTED: r = handleRadiusSelected_; break;
        case RS_UPDATE: r = handleRadiusSelected_+1; break;
        case RS_NORMAL: break;
    }

    QRect rect(0,0,r*2,r*2);
    rect.moveTo(time2screen(p.t) - r,
                value2screen(p.val) - r);
    return rect;

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

    auto it0 = tl_->first(screen2time(i0-handleRadiusSelected_)),
         it1 = tl_->first(screen2time(i1+handleRadiusSelected_));

    for (auto it = it0; it != it1 && it != tl_->getData().end(); ++it)
    {
        const bool hovered = (hoverHash_ == it->first);
        const bool selected = (selectHashSet_.contains(it->first));

        if (selected)
            p.setBrush(QBrush(QColor(200,255,200)));
        else
        if (hovered)
            p.setBrush(QBrush(QColor(250,250,150,200)));
        else
            p.setBrush(QBrush(QColor(200,200,100,200)));

        p.drawRect(handleRect_(it->second, hovered? RS_HOVER : RS_NORMAL));

        if (selected)
        {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(200,255,200)));
            p.drawRect(handleRect_(it->second, RS_SELECTED));
            p.setPen(Qt::NoPen);
        }
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
            update(handleRect_(it1->second, RS_UPDATE));
    }
}

void Timeline1DView::setHover_(const Timeline1D::Point & p)
{
    auto hash = Timeline1D::hash(p.t);

    if (hash == hoverHash_)
        return;

    if (isHover_() && tl_)
    {
        // remove old flag
        auto it1 = tl_->getData().lower_bound(hoverHash_);
        if (it1 != tl_->getData().end())
            update(handleRect_(it1->second, RS_UPDATE));
    }

    hoverHash_ = hash;
    update(handleRect_(p, RS_UPDATE));
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
            update(handleRect_(it1->second, RS_UPDATE));
    }
    selectHashSet_.clear();
}

void Timeline1DView::addSelect_(const Timeline1D::Point & p)
{
    auto hash = Timeline1D::hash(p.t);

    if (selectHashSet_.contains(hash))
        return;

    selectHashSet_.insert(hash);

    update(handleRect_(p, RS_UPDATE));
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

    // --- drag position / space ---

    if (action_ == A_DRAG_SPACE)
    {
        // mouse delta in timeline space
        Double dx = screen2time(e->x()) - screen2time(dragStart_.x()),
               dy = screen2value(e->y()) - screen2value(dragStart_.y());

        space_.offsetX = dragStartSpace_.offsetX + dx;
        space_.offsetY = dragStartSpace_.offsetY + dy;

        update();

        e->accept();
        return;
    }

    // --- drag selected points ---

    if (action_ == A_DRAG_SELECTED)
    {
        // mouse delta in timeline space
        Double dx = screen2time(e->x()) - screen2time(dragStart_.x()),
               dy = screen2value(e->y()) - screen2value(dragStart_.y());

        // only move vertically on CTRL
        if (e->modifiers() & Qt::CTRL)
            dx = 0;

        moveSelected_(dx, dy);

        e->accept();
        return;
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

    // ---- click on point ----

    if (isHover_())
    {
        if (e->button() == Qt::LeftButton)
        {
            auto point = hoverPoint_();
            // fail-safe (should be a point though)
            if (point == tl_->getData().end())
                return;

            // keep selection when shift pressed
            if (!(e->modifiers() & Qt::SHIFT)
                // or when already selected
                && !selectHashSet_.contains(hoverHash_))
                clearSelect_();

            addSelect_(point->second);
            action_ = A_DRAG_SELECTED;
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

    // ----- click in empty space -----

    // drag space / position
    if (e->button() == Qt::LeftButton)
    {
        //if (!(e->modifiers() & Qt::SHIFT))
        clearSelect_();

        action_ = A_DRAG_SPACE;
        dragStart_ = e->pos();
        dragStartSpace_ = space_;
        e->accept();
        return;
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

    selectHashSet_.clear();

    for (auto &p : dragPoints_)
    {
        auto it = tl_->first(p.newp.t);

        if (it == tl_->getData().end())
            continue;

        if (tl_->find(p.oldp.t + dx) != tl_->getData().end())
        {
            qDebug() << "already there";
            continue;
        }

        // delete previous point
        tl_->remove(it->second.t);

        // create a new one
        auto newp = tl_->add(p.oldp.t + dx, p.oldp.val + dy, p.oldp.type);

        if (newp != 0)
        {
            // keep the new position to find it next time
            p.newp = *newp;

            // update selection hash
            selectHashSet_.insert(tl_->hash(newp->t));
        }
    }

    update();
}

} // namespace GUI
} // namespace MO
