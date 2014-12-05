/** @file objectgraphconnectitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include <QCursor>
#include <QGraphicsTextItem>

#include "objectgraphconnectitem.h"
#include "abstractobjectitem.h"
#include "gui/util/objectgraphsettings.h"
#include "gui/util/objectgraphscene.h"
#include "object/audioobject.h"
#include "object/param/parameter.h"

namespace MO {
namespace GUI {

ObjectGraphConnectItem::ObjectGraphConnectItem(uint channel, const QString& toolTip,
                                               AbstractObjectItem *object)
    : QGraphicsEllipseItem  (object),
      objectItem_           (object),
      channel_              (channel),
      param_                (0),
      text_                 (0),
      hovered_            (false)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::ArrowCursor));
    setBrush(ObjectGraphSettings::brushConnector(this));
    setPen(Qt::NoPen);

    setRect(-6, -6, 12, 12);

    setToolTip(toolTip);

    setText(toolTip);
}

ObjectGraphConnectItem::ObjectGraphConnectItem(uint channel, AbstractObjectItem *parent)
    : ObjectGraphConnectItem(channel, QString::number(channel + 1), parent)
{ }

ObjectGraphConnectItem::ObjectGraphConnectItem(Parameter * p, AbstractObjectItem *object)
    : QGraphicsEllipseItem  (object),
      objectItem_           (object),
      channel_              (0),
      param_                (p),
      text_                 (0)
{
    setAcceptHoverEvents(true);

    setCursor(QCursor(Qt::ArrowCursor));
    setBrush(ObjectGraphSettings::brushConnector(this));
    setPen(Qt::NoPen);

    setRect(-6, -6, 12, 12);

    setToolTip(param_->statusTip());
    setText(param_->name());
}

void ObjectGraphConnectItem::setText(const QString & t)
{
    if (text_)
        text_->setPlainText(t);
    else
    {
        text_ = new QGraphicsTextItem(t, this);
        text_->setFont(ObjectGraphSettings::fontConnector());
        // XXX position not clear to me
        text_->setPos(rect().left() + 8,
                      rect().top() - text_->boundingRect().height()/3 );

        text_->setDefaultTextColor(
                    ObjectGraphSettings::colorObjectText(objectItem_->object()));

        setFlag(ItemClipsToShape, false);
    }
}


void ObjectGraphConnectItem::hoverEnterEvent(QGraphicsSceneHoverEvent * )
{
    hovered_ = true;
    update();
}

void ObjectGraphConnectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * )
{
    hovered_ = false;
    update();
}


void ObjectGraphConnectItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    if (auto ao = qobject_cast<AudioObject*>(objectItem_->object()))
        objectItem_->objectScene()->startConnection(ao);

    update();
}

void ObjectGraphConnectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setBrush(ObjectGraphSettings::brushConnector(this));
    QGraphicsEllipseItem::paint(painter, option, widget);
}

} // namespace GUI
} // namespace MO
