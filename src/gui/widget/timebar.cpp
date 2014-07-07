/** @file timebar.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/7/2014</p>
*/

#include <QMouseEvent>

#include "timebar.h"
#include "gui/util/viewspace.h"

namespace MO {
namespace GUI {



TimeBar::TimeBar(QWidget *parent) :
    QWidget (parent),
    time_   (0.0),
    offset_ (0.0)
{
    setFixedWidth(3);
    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(0,255,255, 100));
    setPalette(p);

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

    if (x < 0 || x > rect_.width())
        setVisible(false);
    else
    {
        move(x - 1 + rect_.x(), rect_.y());
        if (!isVisible())
            setVisible(true);
    }
}


void TimeBar::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        timeStart_ = time_;
        dragStart_ = mapToGlobal( e->pos() );
        e->accept();
    }
}

void TimeBar::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        const QPoint g = mapToGlobal( e->pos() );
        const int delta = g.x() - dragStart_.x();

        const Double newTime = std::max((Double)0,
                timeStart_ +
                space_.mapXDistanceTo((Double)delta/rect_.width()) );

        //setTime(newTime);

        emit timeChanged(newTime - offset_);
    }
}


} // namespace GUI
} // namespace MO
