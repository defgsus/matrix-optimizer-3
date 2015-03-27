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
class Parameter;
class Modulator;
namespace GUI {

class ObjectGraphScene;

/** Base class for representing MO::Object in the ObjectGraphScene.
    @note Right now this class is not abstract, but rather handles every
    object type by itself, ... */
class AbstractObjectItem : public QGraphicsItem
{
public:

    // Does not work with qgraphicsitem_cast!!
    enum Type
    {
        T_BASE = UserType + 42
    };

    /** Constructs an AbstractObjectItem for the given object.
        @p object MUST point to a valid object */
    AbstractObjectItem(Object * object, QGraphicsItem * parent = 0);
    ~AbstractObjectItem();

    // -------------------- getter ---------------------

    /** Returns the ObjectGraphScene this item is in, or NULL */
    ObjectGraphScene * objectScene() const;

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
    QSize gridSize() const;

    /** Returns the position in the grid,
        which is derived from the pos(). */
    const QPoint& gridPos() const;

    /** Returns the rectangle occupied in the grid */
    const QRect gridRect() const { return QRect(gridPos(), gridSize()); }

    /** Returns the scene-global position in grid coordinates */
    QPoint globalGridPos() const;

    /** Translates a global point into global grid coordinates */
    QPoint mapToGrid(const QPointF&) const;

    /** Translates global grid coordinates into global coordinates */
    QPointF mapFromGrid(const QPoint&) const;

    /** Returns the pixel rectangle */
    QRectF rect() const;

    /** Returns true if mouse is over item */
    bool isHover() const;

    QList<AbstractObjectItem*> childObjectItems() const;

    /** Returns the item for the given grid-pos, or NULL */
    AbstractObjectItem * childItemAt(const QPoint& gridpos) const;

    QRectF childrenBoundingRect(bool checkVisibilty);
    using QGraphicsItem::childrenBoundingRect;

    // ---------- connector positions ------------------

    /** Returns the audio connector position in local coords */
    QPointF inputPos(uint channel = 0) const;

    /** Returns the connector position in local coords */
    QPointF outputPos(uint channel = 0) const;

    /** Returns the connector position in scene coords */
    QPointF globalInputPos(uint channel = 0) const
        { return mapToScene(inputPos(channel)); }

    /** Returns the connector position in scene coords */
    QPointF globalOutputPos(uint channel = 0) const
        { return mapToScene(outputPos(channel)); }

    /** Returns the parameter's position in local coords.
        If the parameter is not visible some common place is returned. */
    QPointF inputPos(Parameter * p) const;

    /** Returns the position in local coords for outgoing modulation edges. */
    QPointF outputPos(Modulator * p) const;

    /** Returns the channel of the input or output connectors for
        item-local position, or -1 */
    int channelForPosition(const QPointF &localPos);

    // ------------------ setter -----------------------

    /** Sets the item to be expanded/collapsed */
    void setExpanded(bool enable = true);

    /** Sets a new position for the item.
        If @p expand is true,
        expandTopLeft() will be called for the parent if necessary. */
    void setGridPos(const QPoint& pos, bool expand = false);

    /** Sets a new size in grid coords */
    void setGridSize(const QSize& size);

    /** Requests an update of the surrounding layout */
    void setLayoutDirty(bool dirty = true);

    // ----------------- layout stuff ------------------

    /** Updates all brushes/pens from object hue */
    void updateColors();

    /** Currently updates the name label */
    void updateLabels();

    /** Re-creates all connection items */
    void updateConnectors();

    /** Expands the grid size of the item by moving the top-left
        corner. All parent items will be expanded if necessary. */
    void expandTopLeft(int x, int y);

    /** Recursively adjust all sizes to fit in the children */
    void adjustSizeToChildren();

    //void adjustRightItems();

    // ---------- QGraphicsItem interface --------------

    /** Returns the true pixel coordinates resulting from
        gridPos() and gridSize() times the ObjectGraphSettings::gridSize() */
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;

    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

protected:

    void setUnexpandedSize(const QSize&);
    /** Show/hide the expand/collapse item. (may be ignored) */
    void setExpandVisible(bool);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent*) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent*) Q_DECL_OVERRIDE;

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    virtual void dropEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;

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
