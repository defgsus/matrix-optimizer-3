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

#include "ObjectGraphConnectItem.h"
#include "AbstractObjectItem.h"
#include "gui/util/ObjectGraphSettings.h"
#include "gui/util/ObjectGraphScene.h"
#include "object/AudioObject.h"
#include "object/Scene.h"
#include "object/param/Parameter.h"
#include "object/util/ObjectEditor.h"
#include "model/ObjectMimeData.h"
#include "io/error.h"


namespace MO {
namespace GUI {

ObjectGraphConnectItem::ObjectGraphConnectItem(
        bool isInput, SignalType signal, uint channel, const QString& id,
        const QString& toolTip, AbstractObjectItem *object)
    : QGraphicsEllipseItem  (object),
      objectItem_           (object),
      object_               (object->object()),
      isInput_              (isInput),
      textVis_              (true),
      channel_              (channel),
      signalType_           (signal),
      id_                   (id),
      param_                (0),
      text_                 (0),
      hovered_              (false),
      dragHovered_          (false)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::ArrowCursor));
    setBrush(ObjectGraphSettings::brushConnector(this));

    setRadius(6);

    setToolTip(toolTip);
    setText(toolTip);
}

ObjectGraphConnectItem::ObjectGraphConnectItem(
        Parameter * p, AbstractObjectItem *object)
    : QGraphicsEllipseItem  (object),
      objectItem_           (object),
      object_               (object->object()),
      isInput_              (true),
      textVis_              (true),
      channel_              (0),
      signalType_           (p->signalType()),
      id_                   (p->idName()),
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

    setRadius(6);

    updateName();
}

void ObjectGraphConnectItem::setRadius(qreal r)
{
    setRect(-r, -r, r*2., r*2.);
}

void ObjectGraphConnectItem::updateName()
{
    if (param_)
    {
        setToolTip(param_->displayName() + "\n" + param_->statusTip());
        setText(param_->displayName());
    }
}

void ObjectGraphConnectItem::setText(const QString & t)
{
    if (text_)
        text_->setPlainText(t);
    else
    {
        text_ = new QGraphicsTextItem(t, this);
        text_->setFont(ObjectGraphSettings::fontConnector());
        // XXX position offset not clear to me
        text_->setPos(isInput_
                      ? rect().left() + 9
                      : rect().right() - text_->boundingRect().width() - 9,
                      rect().top() - text_->boundingRect().height()/3 );

        text_->setDefaultTextColor(brush().color().lighter());
        //text_->setDefaultTextColor(
        //            ObjectGraphSettings::colorText(objectItem_->object()));
        text_->setVisible(textVis_);

        setFlag(ItemClipsToShape, false);
    }
}

void ObjectGraphConnectItem::setTextVisible(bool vis)
{
    textVis_ = vis;
    if (text_)
        text_->setVisible(vis);
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
            QMessageBox::tr("Can't connect to object from another "
                            "application instance.\nSorry."));
        return;
    }

    if (object_->editor())
    {
        object_->editor()->addModulator(parameter(), desc.pointer()->idName(), "");
        e->accept();
    }
}


void ObjectGraphConnectItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    //if (qobject_cast<AudioObject*>(objectItem_->object()))
        objectItem_->objectScene()->startConnection(this);

    update();
}

void ObjectGraphConnectItem::paint(
        QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
