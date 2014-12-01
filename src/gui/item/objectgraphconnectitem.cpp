/** @file objectgraphconnectitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include <QCursor>

#include "objectgraphconnectitem.h"
#include "abstractobjectitem.h"
#include "gui/util/objectgraphsettings.h"
#include "gui/util/objectgraphscene.h"
#include "object/audioobject.h"

namespace MO {
namespace GUI {

ObjectGraphConnectItem::ObjectGraphConnectItem(AbstractObjectItem *parent)
    : QGraphicsEllipseItem (parent),
      objectItem_   (parent)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::ArrowCursor));
    setBrush(QBrush(ObjectGraphSettings::colorOutline(objectItem_->object(), false)));
    setPen(Qt::NoPen);

    setRect(-6, -6, 12, 12);
}

void ObjectGraphConnectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    setBrush(QBrush(ObjectGraphSettings::colorOutline(objectItem_->object(), true)));
    update();
}

void ObjectGraphConnectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    setBrush(QBrush(ObjectGraphSettings::colorOutline(objectItem_->object(), false)));
    update();
}

void ObjectGraphConnectItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    if (auto ao = qobject_cast<AudioObject*>(objectItem_->object()))
        objectItem_->objectScene()->startConnection(ao);

    update();
}

} // namespace GUI
} // namespace MO
