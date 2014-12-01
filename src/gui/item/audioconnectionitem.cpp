/** @file audioconnectionitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include <QPainter>
#include <QCursor>

#include "audioconnectionitem.h"
#include "abstractobjectitem.h"
#include "gui/util/objectgraphscene.h"
#include "gui/util/objectgraphsettings.h"
#include "object/object.h"
#include "object/audioobject.h"
#include "object/util/audioobjectconnections.h"
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
        QPoint p1(0, -2),
               p2(0, 2);

        QPainterPath shape(from + p1);

        addCubicPath(shape, from + p1, to + p1);
        shape.lineTo(to + p2);
        addCubicPath(shape, to + p2, from + p2, -1.0);

        return shape;
    }
}



AudioConnectionItem::AudioConnectionItem(AudioObjectConnection * con)
    : con_          (con),
      isHovered_    (false),
      from_         (0),
      to_           (0)
{
    setCursor(QCursor(Qt::ArrowCursor));
    setFlag(ItemIsSelectable);
}

ObjectGraphScene * AudioConnectionItem::objectScene() const
{
    return qobject_cast<ObjectGraphScene*>(scene());
}

void AudioConnectionItem::updateShape()
{
    prepareGeometryChange();
    updateFromTo_(); // a connected item's parent might be collapsed/expanded
    updatePos_();
    update();
}


void AudioConnectionItem::updateFromTo_()
{
    from_ = to_ = 0;

    if (con_)
    {
        if (auto s = objectScene())
        {
            from_ = s->visibleItemForObject(con_->from());
            to_ = s->visibleItemForObject(con_->to());
            //MO_DEBUG(mod_->AudioObjectConnection()->name() << " -> " << mod_->parent()->name() << ": "
            //            << from_ << " " << to_);

            // don't show connections to self
            // if they don't belong to the visible object
            setVisible(
                !( from_ == to_
                && con_->from() != from_->object()
                && con_->to() != to_->object()));

        }
    }
}

void AudioConnectionItem::updatePos_()
{
    // get positions of AudioObjectConnection and goal item
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
    boundingShape_ = createArrowOutline(fromPos_ - pos(), toPos_ - pos());
}

QPainterPath AudioConnectionItem::shape() const
{
    return boundingShape_;
}

QRectF AudioConnectionItem::boundingRect() const
{
    return shape_.boundingRect();
}

void AudioConnectionItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    isHovered_ = true;
    update();
}

void AudioConnectionItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    isHovered_ = false;
    update();
}

void AudioConnectionItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    const bool sel = (from_ && from_->isSelected())
                  || (to_ && to_->isSelected());
    const bool act = (!from_ || (from_->object() && from_->object()->activeAtAll()))
                  && (!to_ || (to_->object() && to_->object()->activeAtAll()));

    p->setPen(ObjectGraphSettings::penAudioConnection(con_, sel, isSelected(), act));

    p->drawPath(shape_);
}

//void AudioConnectionItem::mouseMoveEvent()

} // namespace GUI
} // namespace MO
