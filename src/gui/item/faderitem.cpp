/** @file faderitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsScene>

#include "faderitem.h"
#include "io/error.h"

namespace MO {
namespace GUI {

FaderItem::FaderItem(QGraphicsItem * p)
    : AbstractGuiItem   (p)
    , value_            (0.)
    , min_              (0.)
    , max_              (100.)
    , do_drag_          (false)
    , colorOn_          (QColor(100,150,100))
    , colorOff_         (QColor(30,70,30))
    , vertical_         (true)
{
}

void FaderItem::setRange(Float mi, Float ma)
{
    min_ = mi;
    max_ = ma;
    value_ = std::max(min_, std::min(max_, value_ ));
    update();
}

void FaderItem::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        do_drag_ = true;
        drag_start_value_ = value_;

        // select parent
        if (parentItem())
        {
            MO_ASSERT(scene(), "missing scene in QGraphicsItem event");
            scene()->clearSelection();
            parentItem()->setSelected(true);
            // not ideal. should actually single-select parent only
            //scene()->setSelectionArea(parentItem()->mapToScene(parentItem()->shape()),
            //                          Qt::IntersectsItemShape);
        }

        e->accept();
    }
}

void FaderItem::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    if (do_drag_)
    {
        // get mouse delta
        Float dm = vertical_ ?
                      (e->buttonDownScenePos(Qt::LeftButton).y() - e->scenePos().y()) / height()
                  : -((e->buttonDownScenePos(Qt::LeftButton).x() - e->scenePos().x()) / width());

        // get value change factor
        dm *= range();
        if (e->modifiers() && Qt::SHIFT)
            dm /= 10.;

        p_setEmit_(drag_start_value_ + dm);
        e->accept();
    }
}

void FaderItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    do_drag_ = false;
    AbstractGuiItem::mouseReleaseEvent(event);
}

void FaderItem::wheelEvent(QGraphicsSceneWheelEvent * e)
{
    // get value change factor
    Float dm = range() / 500.;
    if (e->modifiers() && Qt::SHIFT)
        dm /= 10.;

    if (e->delta()<0)
        dm = -dm;

    p_setEmit_(value_ + dm);
}

void FaderItem::p_setEmit_(Float v)
{
    Float newValue = std::max(min_,std::min(max_, v ));

    // see if value changed
    bool changed = newValue != value_;
    value_ = newValue;

    if (changed && callback_)
    {
        emit callback_(value_);
        update();
    }
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
