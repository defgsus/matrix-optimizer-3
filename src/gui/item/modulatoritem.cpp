/** @file modulatoritem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

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



namespace {

    void addCubicPath(QPainterPath& shape, const QPointF &from, const QPointF &to, qreal forward = 1.0)
    {
        auto pd = to - from;
        shape.cubicTo(from.x() + forward * std::abs(pd.x()) / 2,    from.y(),
                      to.x() - forward * std::abs(pd.x()) / 2,      to.y(),
                      to.x(), to.y());
    }

    QPainterPath createArrow(const QPointF &from, const QPointF &to)
    {
        QPainterPath shape(from);

        addCubicPath(shape, from, to);

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

    QPainterPath createArrowOutline(const QPointF &from, const QPointF &to)
    {
        QPointF dir = (to - from),
                norm = dir / std::sqrt(std::max(1., QPointF::dotProduct(dir, dir)));

        qreal ny = std::abs(norm.y()),
              ofsy = 3 + 2 * ny;//5 * (ny + (ny+1));

        QPointF p1(0, -ofsy),
                p2(0,  ofsy);

        QPainterPath shape(from + p1);

        addCubicPath(shape, from + p1, to + p1);
        shape.lineTo(to + p2);
        addCubicPath(shape, to + p2, from + p2, -1.0);

        return shape;
    }
}



ModulatorItem::ModulatorItem(Modulator * mod, QGraphicsItem * parent)
    : QGraphicsItem (parent),
      mod_          (mod),
      isHovered_    (false),
      from_         (0),
      to_           (0)
{
    setCursor(QCursor(Qt::ArrowCursor));
    setFlag(ItemIsSelectable);
    setFlag(ItemClipsToShape);

    setToolTip(mod->nameAutomatic());
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

            // don't show connections to self
            // if they don't belong to the visible object
            setVisible( from_ && to_ &&
                !( from_ == to_
                && mod_->modulator() != from_->object()
                && mod_->parent() != to_->object()));

        }
    }
}

void ModulatorItem::updatePos_()
{
    // get positions of modulator and goal item
    if (from_)
        fromPos_ = mapToParent(mapFromScene(
                                   from_->mapToScene(from_->outputPos(mod_))));
    if (to_)
        toPos_ = mapToParent(mapFromScene(
                                 to_->mapToScene(to_->inputPos(mod_->parameter()))));

    // bounding rect of the two positions
    QRect rect( std::min(fromPos_.x(), toPos_.x()),
                std::max(fromPos_.x(), toPos_.x()),
                std::min(fromPos_.y(), toPos_.y()),
                std::max(fromPos_.y(), toPos_.y()) );
    setPos(rect.topLeft());

    // arrow
    shape_ = createArrow(fromPos_ - pos(), toPos_ - pos());
    boundingShape_ = createArrowOutline(fromPos_ - pos(), toPos_ - pos());
    boundingRect_ = boundingShape_.boundingRect();
}

QPainterPath ModulatorItem::shape() const
{
    return boundingShape_;
}

QRectF ModulatorItem::boundingRect() const
{
    return boundingRect_;
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

void ModulatorItem::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::RightButton)
    {
        setSelected(true);
        objectScene()->popup(mod_);
    }
}

void ModulatorItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        setSelected(true);
        objectScene()->openModulatorDialog(mod_);
    }
}

void ModulatorItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    const bool sel = (from_ && from_->isSelected())
                  || (to_ && to_->isSelected());
    const bool act = (!from_ || (from_->object() && from_->object()->activeAtAll()))
                  && (!to_ || (to_->object() && to_->object()->activeAtAll()));
    p->setPen(ObjectGraphSettings::penModulator(mod_, sel, isSelected(), act));

    p->drawPath(shape_);
}

//void ModulatorItem::mouseMoveEvent()

} // namespace GUI
} // namespace MO
