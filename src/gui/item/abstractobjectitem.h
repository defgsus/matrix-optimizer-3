/** @file abstractobjectitem.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#ifndef MOSRC_GUI_ITEM_ABSTRACTOBJECTITEM_H
#define MOSRC_GUI_ITEM_ABSTRACTOBJECTITEM_H

#include <QGraphicsItem>

namespace MO {
class Object;
namespace GUI {

class AbstractObjectItem : public QGraphicsItem
{
public:

    /** Constructs an AbstractObjectItem for the given object.
        @p object MUST point to a valid object */
    AbstractObjectItem(Object * object, QGraphicsItem * parent = 0);
    ~AbstractObjectItem();

    // -------------------- getter ---------------------

    /** Returns the assigned object */
    Object * object() const;

    /** Returns the icon for the assigned object */
    const QIcon & icon() const;

    /** Returns true if item is expanded */
    bool isExpanded() const;

    /** Returns the size of the item in grid-cells.
        This changes with expanded() state. */
    const QSize& gridSize() const;

    /** Returns the position in the grid */
    const QPoint& gridPos() const;

    /** Returns true if mouse is over item */
    bool isHover() const;

    // ------------------ setter -----------------------

    /** Sets the item to be expanded/collapsed */
    void setExpanded(bool enable);

    /** Sets a new position for the item */
    void setGridPos(const QPoint& pos);

    // ---------- QGraphicsItem interface --------------

    /** Returns the true pixel coordinates resulting from
        gridPos() and gridSize() times the ObjectGraphSettings::gridSize() */
    QRectF boundingRect() const Q_DECL_OVERRIDE;

    void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

private:

    class PrivateOI;
    PrivateOI * p_oi_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_ABSTRACTOBJECTITEM_H
