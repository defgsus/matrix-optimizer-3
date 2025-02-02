/** @file objectgraphconnectitem.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_GUI_ITEM_OBJECTGRAPHCONNECTITEM_H
#define MOSRC_GUI_ITEM_OBJECTGRAPHCONNECTITEM_H

#include <QGraphicsItem>

#include "object/Object_fwd.h"

namespace MO {
class Parameter;
class Object;
namespace GUI {

class AbstractObjectItem;

class ObjectGraphConnectItem : public QGraphicsEllipseItem
{
public:

    enum { Type = UserType + 2 };

    ObjectGraphConnectItem(bool isInput, SignalType, uint channel,
                           const QString& id,
                           const QString& toolTip, AbstractObjectItem * parent);

    /** Creates an input for a Parameter */
    ObjectGraphConnectItem(Parameter * , AbstractObjectItem * parent);

    uint channel() const { return channel_; }
    QString id() const { return id_; }
    Object * object() const { return object_; }
    AbstractObjectItem * objectItem() const { return objectItem_; }

    Parameter * parameter() const { return param_; }

    void setRadius(qreal);
    void setText(const QString&);
    void setTextVisible(bool vis);

    bool isInput() const { return isInput_; }
    bool isOutput() const { return !isInput_; }
    bool isParameter() const { return param_; }
    SignalType signalType() const { return signalType_; }

    bool isHovered() const { return hovered_; }

    bool acceptsModulator(Object *) const;

    /** If tied to a parameter or named output, updates via setText() */
    void updateName();

    // ---------- QGraphicsItem interface --------------

    virtual int type() const Q_DECL_OVERRIDE { return Type; }

protected:

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    virtual void dropEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;

    void hoverEnterEvent(QGraphicsSceneHoverEvent*) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) Q_DECL_OVERRIDE;

    void mousePressEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) Q_DECL_OVERRIDE;

private:

    AbstractObjectItem * objectItem_;
    Object * object_;

    bool isInput_, textVis_;
    uint channel_;
    SignalType signalType_;
    QString id_;

    Parameter * param_;

    QGraphicsTextItem * text_;

    bool hovered_, dragHovered_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_OBJECTGRAPHCONNECTITEM_H
