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

    // Does not work with qgraphicsitem_cast!!
    enum Type
    {
        T_BASE = UserType + 1
    };

    /** Constructs an AbstractObjectItem for the given object.
        @p object MUST point to a valid object */
    AbstractObjectItem(Object * object, QGraphicsItem * parent = 0);
    ~AbstractObjectItem();

    // -------------------- getter ---------------------

    virtual int type() const { return T_BASE; }

    /** Returns the assigned object */
    Object * object() const;

    /** Returns the icon for the assigned object */
    const QIcon & icon() const;

    /** Returns the parent AbstractObjectItem, or NULL */
    AbstractObjectItem * parentObjectItem() const;

    /** Returns true if item is expanded */
    bool isExpanded() const;

    /** Returns true when size or position have changed
        and the surrounding layout may need adjustment */
    bool isLayouted() const;

    /** Returns the size of the item in grid-cells.
        This changes with expanded() state. */
    const QSize& gridSize() const;

    /** Returns the position in the grid,
        which is derived from the pos(). */
    const QPoint& gridPos() const;

    /** Returns the rectangle occupied in the grid */
    const QRect gridRect() const { return QRect(gridPos(), gridSize()); }

    /** Translates a global point into grid coordinates */
    QPoint mapToGrid(const QPointF&) const;

    /** Translates grid coordinates into global coordinates */
    QPointF mapFromGrid(const QPoint&) const;

    /** Returns the pixel rectangle */
    QRectF rect() const;

    /** Returns true if mouse is over item */
    bool isHover() const;

    /** Returns the item for the given grid-pos, or NULL */
    AbstractObjectItem * itemInGrid(const QPoint& gridpos) const;

    // ------------------ setter -----------------------

    /** Sets the item to be expanded/collapsed */
    void setExpanded(bool enable);

    /** Sets a new position for the item */
    void setGridPos(const QPoint& pos);

    /** Sets a new size in grid coords */
    void setGridSize(const QSize& size);

    /** Sets an offset to the actual position */
    void setGridOffset(const QPoint& offset);

    /** Requests an update of the surrounding layout */
    void setLayoutDirty(bool dirty = true);

    // ----------------- layout stuff ------------------

    /** Recursively adjust all sizes to fit in the children */
    void adjustSizeToChildren();

    void adjustRightItems();

    // ---------- QGraphicsItem interface --------------

    /** Returns the true pixel coordinates resulting from
        gridPos() and gridSize() times the ObjectGraphSettings::gridSize() */
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*) Q_DECL_OVERRIDE;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;

private:

    class PrivateOI;
    PrivateOI * p_oi_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_ABSTRACTOBJECTITEM_H
