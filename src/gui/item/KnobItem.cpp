/** @file knobitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.03.2015</p>
*/

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsScene>

#include "KnobItem.h"
#include "io/error.h"

namespace MO {
namespace GUI {

KnobItem::KnobItem(QGraphicsItem * p)
    : AbstractGuiItem   (p)
    , value_            (0.)
    , defValue_         (0.)
    , min_              (0.)
    , max_              (100.)
    , do_drag_          (false)
    , colorOn_          (QColor(100,150,100))
    , colorOff_         (QColor(30,70,30))
{
}

void KnobItem::setRange(Float mi, Float ma)
{
    min_ = mi;
    max_ = ma;
    value_ = std::max(min_, std::min(max_, value_ ));
    update();
}

void KnobItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*e)
{
    p_setEmit_(defaultValue());
    QGraphicsItem::mouseDoubleClickEvent(e);
}

void KnobItem::mousePressEvent(QGraphicsSceneMouseEvent * e)
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

void KnobItem::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    if (do_drag_)
    {
        // get mouse delta
        Float dm = (e->buttonDownScenePos(Qt::LeftButton).y() - e->scenePos().y()) / 100.;

        // get value change factor
        dm *= range();
        if (e->modifiers() && Qt::SHIFT)
            dm /= 10.;

        p_setEmit_(drag_start_value_ + dm);
        e->accept();
    }
}

void KnobItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    do_drag_ = false;
    AbstractGuiItem::mouseReleaseEvent(event);
}

void KnobItem::wheelEvent(QGraphicsSceneWheelEvent * e)
{
    // get value change factor
    Float dm = range() / 500.;
    if (e->modifiers() && Qt::SHIFT)
        dm /= 10.;

    if (e->delta()<0)
        dm = -dm;

    p_setEmit_(value_ + dm);
}

void KnobItem::p_setEmit_(Float v)
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

int KnobItem::angle_(Float v) const
{
    return (225.f - v * 270.f) * 16;
}

void KnobItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    Float v = (value_ - min_) / (max_ - min_);
    int v0 = angle_(0),
        v1 = angle_(v);
    const int pw = 12;

    QRectF r(rect());
    r.adjust(pw/2, pw/2, -pw/2, -pw/2);

    p->setBrush(Qt::NoBrush);

    QPen pen(colorOff_);
    pen.setWidth(pw);
    p->setPen(pen);
    p->drawEllipse(r);

    pen.setColor(colorOn_);
    p->setPen(pen);
    p->drawArc(r, v0, v1-v0);
}


} // namespace GUI
} // namespace MO
