/** @file timebar.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/7/2014</p>
*/
#include <QDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

#include "timebar.h"
#include "gui/util/viewspace.h"

namespace MO {
namespace GUI {



TimeBar::TimeBar(QWidget *parent) :
    QWidget (parent),
    time_   (0.0),
    minTime_(-1.0e100),
    maxTime_(1.0e100),
    offset_ (0.0),
    dragging_(false)
{
    setStatusTip("Drag left and right to change time");

    setFixedWidth(3);

    brushBar_ = QBrush(QColor(0,255,255,100));

    if (parent)
        setContainingRect( parent->rect() );

    setCursor(Qt::SplitHCursor);
}

void TimeBar::setContainingRect(const QRect &containing_rect)
{
    rect_ = containing_rect;
    if (rect_.width() < 1)
        rect_.setWidth(1);
    setFixedHeight(rect_.height());
    update_();
}

bool TimeBar::isInContainingRect() const
{
    return geometry().x() >= rect_.left()
            && geometry().x() <= rect_.right();
}

void TimeBar::setViewspace(const UTIL::ViewSpace &space)
{
    space_ = space;
    update_();
}

void TimeBar::setTime(Double time)
{
    time_ = time + offset_;
    update_();
}

void TimeBar::setTimeOffset(Double offset)
{
    time_ = time_ - offset_ + offset;
    offset_ = offset;
    update_();
}

void TimeBar::update_()
{
    const int x = space_.mapXFrom(time_) * rect_.width();

    move(x - 1 + rect_.x(), rect_.y());

    if (!isVisible() && isInContainingRect())
        setVisible(true);
}


void TimeBar::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        dragging_ = true;
        timeStart_ = time_;
        dragStart_ = mapToGlobal( e->pos() );
        e->accept();
    }
}

void TimeBar::mouseMoveEvent(QMouseEvent * e)
{
    if (dragging_)
    {
        const QPoint g = mapToGlobal( e->pos() );
        const int delta = g.x() - dragStart_.x();

        const Double newTime = std::min(maxTime_, std::max(minTime_,
                timeStart_ +
                space_.mapXDistanceTo((Double)delta/rect_.width()) ));

        //setTime(newTime);

        emit timeChanged(newTime - offset_);

        e->accept();
    }
}

void TimeBar::mouseReleaseEvent(QMouseEvent * e)
{
    if (dragging_)
    {
        dragging_ = false;
        e->accept();
    }

    if (!isInContainingRect())
        setVisible(false);
}

void TimeBar::paintEvent(QPaintEvent * e)
{
    // draw only if inside the contained-rect
    if (geometry().left() >= rect_.x() - 1
            && geometry().right() <= rect_.right() + 1)
    {
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(brushBar_);
        p.drawRect(e->rect());
    }
}

} // namespace GUI
} // namespace MO
