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
        Float dm = e->buttonDownScenePos(Qt::LeftButton).y() - e->scenePos().y();

        value_ = drag_start_value_ + dm;

        update();
        e->accept();
    }
}

void FaderItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    do_drag_ = false;
}

void FaderItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    p->setPen(Qt::NoPen);

    QRectF r = rect();
    qreal y = (1.0 - (value_ - min_) / (max_ - min_)) * r.height();

    p->setBrush(QBrush(QColor(50,50,0)));
    p->drawRect(r.left(), r.top(), r.width(), y);
    p->setBrush(QBrush(Qt::yellow));
    p->drawRect(r.left(), r.top() + y, r.width(), r.height() - y);
}


} // namespace GUI
} // namespace MO
