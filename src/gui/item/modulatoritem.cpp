/** @file modulatoritem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#include <QPainter>

#include "modulatoritem.h"
#include "abstractobjectitem.h"
#include "gui/util/objectgraphscene.h"
#include "object/object.h"
#include "object/param/modulator.h"
#include "io/log.h"

namespace MO {
namespace GUI {



ModulatorItem::ModulatorItem(Modulator * mod)
    : mod_          (mod),
      isHovered_    (false),
      from_         (0),
      to_           (0)
{
}

ObjectGraphScene * ModulatorItem::objectScene() const
{
    return qobject_cast<ObjectGraphScene*>(scene());
}

void ModulatorItem::updateShape()
{
    prepareGeometryChange();
    updateFromTo_();
    update();
}

void ModulatorItem::updateFromTo_()
{
    from_ = to_ = 0;

    if (mod_ && mod_->modulator())
    {
        if (auto s = objectScene())
        {
            from_ = s->visibleItemForObject(mod_->modulator());
            to_ = s->visibleItemForObject(mod_->parent());
            //MO_DEBUG(mod_->modulator()->name() << " -> " << mod_->parent()->name() << ": "
            //            << from_ << " " << to_);
        }
    }

    // get positions of modualtor and goal item
    if (from_)
        fromPos_ = from_->mapToScene(QPointF(from_->rect().width(),
                                             from_->rect().height()/2));
    if (to_)
        toPos_ = to_->mapToScene(QPointF(0, to_->rect().height()/2));

    // min/max rect
    rect_.setLeft( std::min(fromPos_.x(), toPos_.x()) );
    rect_.setRight( std::max(fromPos_.x(), toPos_.x()) );
    rect_.setTop( std::min(fromPos_.y(), toPos_.y()) );
    rect_.setBottom( std::max(fromPos_.y(), toPos_.y()) );
    setPos(rect_.topLeft());

    // arrow
    shape_ = QPainterPath(fromPos_ - pos());
    shape_.lineTo(toPos_ - pos());
}

/*QPainterPath ModulatorItem::shape() const
{
    return shape_;
}*/

QRectF ModulatorItem::boundingRect() const
{
    return QRectF(0, 0, rect_.width(), rect_.height());
}

void ModulatorItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    isHovered_ = true;
    update();
}

void ModulatorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    isHovered_ = false;
    update();
}

void ModulatorItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    p->setPen(QPen(QColor(130,255,130)));

    p->drawPath(shape_);
}

//void ModulatorItem::mouseMoveEvent()
} // namespace GUI
} // namespace MO
