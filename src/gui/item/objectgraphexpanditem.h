/** @file objectgraphexpanditem.h

    @brief Expand/Collapse icon for AbstractObjectItem

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#ifndef MOSRC_GUI_ITEM_OBJECTGRAPHEXPANDITEM_H
#define MOSRC_GUI_ITEM_OBJECTGRAPHEXPANDITEM_H

#include <QGraphicsItem>

namespace MO {
namespace GUI {

class AbstractObjectItem;

class ObjectGraphExpandItem : public QGraphicsItem
{
public:

    ObjectGraphExpandItem(AbstractObjectItem * parent);

    // ---------- QGraphicsItem interface --------------

    virtual int type() const Q_DECL_OVERRIDE { return UserType + 1; }

    QRectF boundingRect() const Q_DECL_OVERRIDE;

    void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

    void mousePressEvent(QGraphicsSceneMouseEvent*);

private:

    AbstractObjectItem * objectItem_;
    bool isHovered_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_ITEM_OBJECTGRAPHEXPANDITEM_H
