/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/4/2016</p>
*/

#include <QPainter>
#include <QCursor>

#include "ObjectGraphControlItem.h"
#include "AbstractObjectItem.h"
#include "gui/util/ObjectGraphSettings.h"
#include "object/Object.h"
#include "io/log.h"

namespace MO {
namespace GUI {

ObjectGraphControlItem::ObjectGraphControlItem(AbstractObjectItem *parent)
    : QGraphicsPixmapItem   (parent)
    , objectItem_           (parent)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::ArrowCursor));
}

void ObjectGraphControlItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    if (func_)
        func_();
}

void ObjectGraphControlItem::paint(
        QPainter *p, const QStyleOptionGraphicsItem* style, QWidget* widget)
{
    QGraphicsPixmapItem::paint(p, style, widget);
}


} // namespace GUI
} // namespace MO

