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

#include "FaderItem.h"
#include "io/error.h"

namespace MO {
namespace GUI {

FaderItem::FaderItem(QGraphicsItem * p)
    : AbstractGuiItem   (p)
    , value_            (0.)
    , defValue_         (0.)
    , min_              (0.)
    , max_              (100.)
    , do_drag_          (false)
    , colorOn_          (QColor(100,150,100))
    , colorOff_         (QColor(30,70,30))
    , vertical_         (true)
    , p_controllable_   (true)
{
}

void FaderItem::setRange(Float mi, Float ma)
{
    min_ = mi;
    max_ = ma;
    value_ = std::max(min_, std::min(max_, value_ ));
    update();
}

void FaderItem::setValue(Float v)
{
    value_ = std::max(min_,std::min(max_, v ));
    update();
}

void FaderItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*e)
{
    if (!p_controllable_)
        return;

    p_setEmit_(defaultValue());
    AbstractGuiItem::mouseDoubleClickEvent(e);
}

void FaderItem::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (!p_controllable_)
        return;

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
        return;
    }

    AbstractGuiItem::mousePressEvent(e);
}

void FaderItem::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    if (!p_controllable_)
        return;

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
        return;
    }

    AbstractGuiItem::mouseMoveEvent(e);
}

void FaderItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!p_controllable_)
        return;

    do_drag_ = false;
    AbstractGuiItem::mouseReleaseEvent(event);
}

void FaderItem::wheelEvent(QGraphicsSceneWheelEvent * e)
{
    if (!p_controllable_)
        return;

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

    Float delta = std::max(0.00001f, max_ - min_);

    if (vertical_)
    {
        qreal y = std::max(0.f,std::min(1.f,
                    Float(1) - (value_ - min_) / delta )) * r.height();

        if (y > 0)
        {
            p->setBrush(QBrush(colorOff_));
            p->drawRect(r.left(), r.top(), r.width(), y);
        }
        if (y < r.height())
        {
            p->setBrush(QBrush(colorOn_));
            p->drawRect(r.left(), r.top() + y, r.width(), r.height() - y);
        }
    }
    else
    {
        qreal x = std::max(0.f,std::min(1.f,
                    (value_ - min_) / delta )) * r.width();

        if (x > 0.)
        {
            p->setBrush(QBrush(colorOn_));
            p->drawRect(r.left(), r.top(), x, r.height());
        }
        if (x < r.width())
        {
            p->setBrush(QBrush(colorOff_));
            p->drawRect(r.left() + x, r.top(), r.width() - x, r.height());
        }
    }
}


} // namespace GUI
} // namespace MO
