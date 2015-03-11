/** @file objectgraphconnectitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#include <QCursor>
#include <QGraphicsTextItem>
#include <QGraphicsSceneDragDropEvent>
#include <QMessageBox>

#include "objectgraphconnectitem.h"
#include "abstractobjectitem.h"
#include "gui/util/objectgraphsettings.h"
#include "gui/util/objectgraphscene.h"
#include "object/audioobject.h"
#include "object/scene.h"
#include "object/param/parameter.h"
#include "object/util/objecteditor.h"
#include "model/objectmimedata.h"
#include "io/error.h"


namespace MO {
namespace GUI {

ObjectGraphConnectItem::ObjectGraphConnectItem(bool isInput, uint channel, const QString& toolTip,
                                               AbstractObjectItem *object)
    : QGraphicsEllipseItem  (object),
      objectItem_           (object),
      object_               (object->object()),
      isInput_              (isInput),
      channel_              (channel),
      param_                (0),
      text_                 (0),
      hovered_              (false),
      dragHovered_          (false)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::ArrowCursor));
    setBrush(ObjectGraphSettings::brushConnector(this));


    setRect(-6, -6, 12, 12);

    setToolTip(toolTip);

    setText(toolTip);
}

ObjectGraphConnectItem::ObjectGraphConnectItem(bool isInput, Parameter * p, AbstractObjectItem *object)
    : QGraphicsEllipseItem  (object),
      objectItem_           (object),
      object_               (object->object()),
      isInput_              (isInput),
      channel_              (0),
      param_                (p),
      text_                 (0),
      hovered_              (false),
      dragHovered_          (false)
{
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

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
        text_->setPos(rect().left() + 9,
                      rect().top() - text_->boundingRect().height()/3 );

        text_->setDefaultTextColor(
                    ObjectGraphSettings::colorText(objectItem_->object()));

        setFlag(ItemClipsToShape, false);
    }
}

bool ObjectGraphConnectItem::acceptsModulator(Object * o) const
{
    return (isParameter()
            && parameter()->object() != o)
            && (parameter()->getModulatorTypes() & o->type()
            // && and self-connection!
            );
}

void ObjectGraphConnectItem::hoverEnterEvent(QGraphicsSceneHoverEvent * )
{
    hovered_ = true;
    update();
}

void ObjectGraphConnectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * )
{
    hovered_ = false;
    dragHovered_ = false;
    update();
}


void ObjectGraphConnectItem::dragEnterEvent(QGraphicsSceneDragDropEvent * e)
{
    if (e->mimeData()->formats().contains(ObjectMimeData::mimeTypeString))
    {
        // avoid self-drop
        auto data = static_cast<const ObjectMimeData*>(e->mimeData());
        if (data->getDescription().pointer() == object()
            || !acceptsModulator(data->getDescription().pointer()))
            return;

        dragHovered_ = true;
        e->accept();
        update();
    }
}

void ObjectGraphConnectItem::dragLeaveEvent(QGraphicsSceneDragDropEvent * )
{
    if (dragHovered_)
    {
        dragHovered_ = false;
        update();
    }
}


void ObjectGraphConnectItem::dropEvent(QGraphicsSceneDragDropEvent * e)
{
    // analyze mime data
    MO_ASSERT(e->mimeData()->formats().contains(ObjectMimeData::mimeTypeString),
              "unaccepted dropevent");

    // construct a wrapper
    auto data = static_cast<const ObjectMimeData*>(e->mimeData());
    auto desc = data->getDescription();

    // analyze further
    if (!desc.isSameApplicationInstance())
    {
        QMessageBox::information(0,
                                 QMessageBox::tr("drop object"),
                                 QMessageBox::tr("Can't drop an object from another application instance."));
        return;
    }

    if (object_->editor())
        object_->editor()->addModulator(parameter(), desc.pointer()->idName(), "");
}


void ObjectGraphConnectItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    if (qobject_cast<AudioObject*>(objectItem_->object()))
        objectItem_->objectScene()->startConnection(this);

    update();
}

void ObjectGraphConnectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setBrush(ObjectGraphSettings::brushConnector(this));
    if (dragHovered_)
        setPen(QPen(Qt::white));
    else
        setPen(Qt::NoPen);

    QGraphicsEllipseItem::paint(painter, option, widget);
}

} // namespace GUI
} // namespace MO
