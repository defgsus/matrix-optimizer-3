/** @file objectgraphscene.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#ifndef MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H
#define MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H

#include <QGraphicsScene>

namespace MO {
class Object;
namespace GUI {

class AbstractObjectItem;
class SceneSettings;

class ObjectGraphScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ObjectGraphScene(QObject *parent = 0);
    ~ObjectGraphScene();

    // -------------- getter -------------------

    /** Returns installed settings, or NULL */
    SceneSettings * guiSettings() const;

    /** Return the item for the object, or NULL */
    AbstractObjectItem * itemForObject(Object * o) const;

    /** Return the first visible item or one of it's parents for the object, or NULL */
    AbstractObjectItem * visibleItemForObject(Object * o) const;

    /** Returns a list of all selected items that are AbstractObjectItems */
    QList<AbstractObjectItem*> selectedObjectItems() const;

    // -------------- coords -------------------

    /** Translates a global point into grid coordinates */
    QPoint mapToGrid(const QPointF& global_pos) const;

    /** Translates grid coordinates into global coordinates */
    QPointF mapFromGrid(const QPoint&) const;

signals:

    void shiftView(const QPointF& delta);

public slots:

    /** Sets the gui-settings for the associated objects */
    void setGuiSettings(SceneSettings * set);

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

    /** Moves the given item to the given position */
    void setGridPos(AbstractObjectItem *, const QPoint& gridPos);

    /** Calls update for all ModulatorItems connected to the given item */
    void repaintModulators(AbstractObjectItem *);

    /** Bring the item and all it's cables to front */
    void toFront(AbstractObjectItem *);

    /** Creates the edit menu, for scene or for selected items */
    void popup(const QPoint &gridPos);

    // ------------------- editing --------------------

    /** Adds an object to object-tree and item-tree.
        @p gridPos is global and will be retranslated to parent's local coords if necessary. */
    void addObject(Object * parent, Object * newObject, const QPoint &gridPos = QPoint(1,1), int insert_index = -1);

private slots:

    void onChanged_();

protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

    virtual void drawForeground(QPainter * p, const QRectF &rect) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H
