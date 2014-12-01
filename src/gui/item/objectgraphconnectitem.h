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

    ObjectGraphConnectItem(AbstractObjectItem * parent);

    // ---------- QGraphicsItem interface --------------

    virtual int type() const Q_DECL_OVERRIDE { return UserType + 2; }

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

    void mousePressEvent(QGraphicsSceneMouseEvent*);

private:

    AbstractObjectItem * objectItem_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_OBJECTGRAPHCONNECTITEM_H
