/** @file objectgraphconnectitem.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_GUI_ITEM_OBJECTGRAPHCONNECTITEM_H
#define MOSRC_GUI_ITEM_OBJECTGRAPHCONNECTITEM_H

#include <QGraphicsItem>

namespace MO {
class Parameter;
namespace GUI {

class AbstractObjectItem;

class ObjectGraphConnectItem : public QGraphicsEllipseItem
{
public:

    enum { Type = UserType + 2 };

    ObjectGraphConnectItem(uint channel, AbstractObjectItem * parent);
    ObjectGraphConnectItem(uint channel, const QString& toolTip, AbstractObjectItem * parent);

    ObjectGraphConnectItem(Parameter * , AbstractObjectItem * parent);

    uint channel() const { return channel_; }

    Parameter * parameter() const { return param_; }

    void setText(const QString&);

    bool isAudioConnector() const { return !param_; }
    bool isParameter() const { return param_; }

    bool isHovered() const { return hovered_; }

    // ---------- QGraphicsItem interface --------------

    virtual int type() const Q_DECL_OVERRIDE { return Type; }

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

    void mousePressEvent(QGraphicsSceneMouseEvent*);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:

    AbstractObjectItem * objectItem_;

    uint channel_;

    Parameter * param_;

    QGraphicsTextItem * text_;

    bool hovered_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_OBJECTGRAPHCONNECTITEM_H
