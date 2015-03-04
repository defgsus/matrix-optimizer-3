/** @file faderitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include "faderitem.h"

namespace MO {
namespace GUI {

FaderItem::FaderItem(QGraphicsItem * p)
    : AbstractGuiItem   (p)
    , do_drag_          (false)
    , colorOn_          (QColor(100,150,100))
    , colorOff_         (QColor(30,70,30))
    , vertical_         (true)
{
    min_ = 0.; max_ = 100.;
    value_ = 25.;
}

void FaderItem::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        do_drag_ = true;
        drag_start_value_ = value_;

        e->accept();
    }
}

void FaderItem::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    if (do_drag_)
    {
        Float dm = vertical_ ?
                      e->buttonDownScenePos(Qt::LeftButton).y() - e->scenePos().y()
                  : -(e->buttonDownScenePos(Qt::LeftButton).x() - e->scenePos().x());

        Float newValue = std::max(min_,std::min(max_,
                        drag_start_value_ + dm ));

        // see if value changed
        bool changed = newValue != value_;
        value_ = newValue;

        if (changed && callback_)
            emit callback_(value_);

        update();
        e->accept();
    }
}

void FaderItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    do_drag_ = false;
    AbstractGuiItem::mouseReleaseEvent(event);
}

void FaderItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    p->setPen(Qt::NoPen);

    QRectF r = rect();

    if (vertical_)
    {
        qreal y = (1.0 - (value_ - min_) / (max_ - min_)) * r.height();

        p->setBrush(QBrush(colorOff_));
        p->drawRect(r.left(), r.top(), r.width(), y);
        p->setBrush(QBrush(colorOn_));
        p->drawRect(r.left(), r.top() + y, r.width(), r.height() - y);
    }
    else
    {
        qreal x = (value_ - min_) / (max_ - min_) * r.width();

        p->setBrush(QBrush(colorOn_));
        p->drawRect(r.left(), r.top(), x, r.height());
        p->setBrush(QBrush(colorOff_));
        p->drawRect(r.left() + x, r.top(), r.width() - x, r.height());
    }
}


} // namespace GUI
} // namespace MO
