/** @file modulatoritem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#include <QPainter>
#include <QCursor>

#include "modulatoritem.h"
#include "abstractobjectitem.h"
#include "gui/util/objectgraphscene.h"
#include "gui/util/objectgraphsettings.h"
#include "object/object.h"
#include "object/param/modulator.h"
#include "math/vector.h"
#include "io/log.h"

namespace MO {
namespace GUI {



ModulatorItem::ModulatorItem(Modulator * mod)
    : mod_          (mod),
      isHovered_    (false),
      from_         (0),
      to_           (0)
{
    setCursor(QCursor(Qt::ArrowCursor));
    setFlag(ItemIsSelectable);
}

ObjectGraphScene * ModulatorItem::objectScene() const
{
    return qobject_cast<ObjectGraphScene*>(scene());
}

void ModulatorItem::updateShape()
{
    prepareGeometryChange();
    updateFromTo_(); // a connected item's parent might be collapsed/expanded
    updatePos_();
    update();
}

QPainterPath ModulatorItem::createArrow(const QPointF &from, const QPointF &to)
{
    QPainterPath shape(from);
    auto pd = to - from;
    //shape_.lineTo(p);
    shape.cubicTo(from.x() + std::abs(pd.x()) / 2,    from.y(),
                  to.x() - std::abs(pd.x()) / 2,      to.y(),
                  to.x(), to.y());

    // arrow head
#if (0) // arrow head in linear direction of path
    const Vec2
            dir = glm::normalize(Vec2(pd.x(), pd.y())) * 10.f,
            p1 = MATH::rotate(dir, 140),
            p2 = MATH::rotate(dir, 220);
    shape.lineTo(to.x() + p1.x,
                 to.y() + p1.y);
    shape.lineTo(to.x() + p2.x,
                 to.y() + p2.y);
#else // arrow head
    shape.lineTo(to + QPointF(-5,-5));
    shape.lineTo(to + QPointF(-5,5));
#endif
    shape.lineTo(to);

    return shape;
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
}

void ModulatorItem::updatePos_()
{
    // get positions of modulator and goal item
    if (from_)
        fromPos_ = from_->mapToScene(QPointF(from_->rect().right(),
                                             from_->rect().center().y()));
    if (to_)
        toPos_ = to_->mapToScene(QPointF(to_->rect().left(), to_->rect().center().y()));

    // min/max rect
    rect_.setLeft(   std::min(fromPos_.x(), toPos_.x()) );
    rect_.setRight(  std::max(fromPos_.x(), toPos_.x()) );
    rect_.setTop(    std::min(fromPos_.y(), toPos_.y()) );
    rect_.setBottom( std::max(fromPos_.y(), toPos_.y()) );
    setPos(rect_.topLeft());

    // arrow
    shape_ = createArrow(fromPos_ - pos(), toPos_ - pos());
}

QPainterPath ModulatorItem::shape() const
{
    return shape_;
}

QRectF ModulatorItem::boundingRect() const
{
    return shape_.boundingRect();
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
    const bool sel = (from_ && from_->isSelected())
                  || (to_ && to_->isSelected());
    const bool act = !from_ || (from_->object() && from_->object()->activeAtAll());
    p->setPen(ObjectGraphSettings::penModulator(mod_, sel, isSelected(), act));

    p->drawPath(shape_);
}

//void ModulatorItem::mouseMoveEvent()

} // namespace GUI
} // namespace MO
