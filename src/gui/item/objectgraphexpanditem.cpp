/** @file objectgraphexpanditem.cpp

    @brief Expand/Collapse icon for AbstractObjectItem

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QPainter>

#include "objectgraphexpanditem.h"
#include "abstractobjectitem.h"
#include "gui/util/objectgraphsettings.h"

namespace MO {
namespace GUI {

ObjectGraphExpandItem::ObjectGraphExpandItem(AbstractObjectItem *parent)
    : QGraphicsItem (parent),
      objectItem_   (parent),
      isHovered_    (false)
{
}

void ObjectGraphExpandItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    isHovered_ = true;
    update();
}

void ObjectGraphExpandItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    isHovered_ = false;
    update();
}

void ObjectGraphExpandItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    objectItem_->setExpanded(!objectItem_->isExpanded());
    update();
}

QRectF ObjectGraphExpandItem::boundingRect() const
{
    const auto s = ObjectGraphSettings::expandItemSize();
    return QRectF(pos_.x(), pos_.y(), s.width(), s.height());
}

void ObjectGraphExpandItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    const bool isExp = objectItem_->isExpanded();

    const auto s = ObjectGraphSettings::expandItemSize();


}


} // namespace GUI
} // namespace MO
