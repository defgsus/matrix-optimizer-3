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
namespace GUI {

class AbstractObjectItem;

class ObjectGraphConnectItem : public QGraphicsEllipseItem
{
public:

    enum { Type = UserType + 2 };

    ObjectGraphConnectItem(uint channel, AbstractObjectItem * parent);
    ObjectGraphConnectItem(uint channel, const QString& toolTip, AbstractObjectItem * parent);

    uint channel() const { return channel_; }

    // ---------- QGraphicsItem interface --------------

    virtual int type() const Q_DECL_OVERRIDE { return Type; }

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

    void mousePressEvent(QGraphicsSceneMouseEvent*);

private:

    AbstractObjectItem * objectItem_;

    uint channel_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_OBJECTGRAPHCONNECTITEM_H
