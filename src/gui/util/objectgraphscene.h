/** @file objectgraphscene.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#ifndef MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H
#define MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H

#include <QGraphicsScene>

class QMimeData;

namespace MO {
class Object;
class AudioObject;
class Parameter;
class Modulator;
class AudioObjectConnection;
class ActionList;
namespace GUI {

class AbstractObjectItem;
class ObjectGraphConnectItem;
class SceneSettings;

class ObjectGraphScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ObjectGraphScene(QObject *parent = 0);
    ~ObjectGraphScene();

    // -------------- getter -------------------

    /** Returns the item for the object, or NULL */
    AbstractObjectItem * itemForObject(const Object *o) const;

    /** Returns the first visible item or one of it's parents for the object, or NULL */
    AbstractObjectItem * visibleItemForObject(Object * o) const;

    /** Returns the item at global grid position, or NULL */
    AbstractObjectItem * objectItemAt(const QPoint& globalPos);

    /** Returns the child item inside @p parent, or NULL.
        If @p parent is NULL, the objectItemAt(const QPoint&) method will be used. */
    AbstractObjectItem * objectItemAt(AbstractObjectItem * parent, const QPoint& posInParent);

    // -------------- coords -------------------

    /** Translates a global point into grid coordinates */
    QPoint mapToGrid(const QPointF& global_pos) const;

    /** Translates grid coordinates into global coordinates */
    QPointF mapFromGrid(const QPoint&) const;

    /** Returns the next free position inside parent */
    QPoint nextFreePosition(Object * parent, const QPoint& localPos) const;
    QPoint nextFreePosition(AbstractObjectItem * item, const QPoint& localPos) const;

    // -------------- item getter --------------

    /** Returns all top-level AbstractObjectItems */
    QList<AbstractObjectItem*> topLevelObjectItems() const;

    /** Returns a list of all selected items that are AbstractObjectItems */
    QList<AbstractObjectItem*> selectedObjectItems() const;

    /** Reduces the objects in the list to the ones at the top */
    static void reduceToTopLevel(QList<AbstractObjectItem*>&);

signals:

    void shiftView(const QPointF& delta);

    /** An object has been selected */
    void objectSelected(MO::Object *);

    /** Actions for edit-mainmenu have been populated. */
    void editActionsChanged(const ActionList&);

public slots:

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

    /** Calls update for all ModulatorItems connected to the given item */
    void repaintModulators(AbstractObjectItem *);

    /** Bring the item and all it's cables to front */
    void toFront(QGraphicsItem *);

    /** Creates edit actions for current selection and sends it to MainWidgetController */
    void createEditMenu();

    /** Creates the edit menu, for scene or for selected items */
    void popup(const QPoint &gridPos);

    /** Popup for connection */
    void popup(MO::AudioObjectConnection*);
    /** Popup for modulator */
    void popup(MO::Modulator*);
    /** Popup with parameter select to create a modulation.
        @p dropPoint is in scene coords */
    void popupObjectDrag(MO::Object * source, MO::Object * goal, const QPointF &dropPoint);

    /** Focus and select the item for the object. */
    void setFocusObject(MO::Object * o);

    /** Starts the connection drag mode */
    bool startConnection(ObjectGraphConnectItem*);

    // ------------------- clipboard ------------------

    /** Creates the QMimeData of the items in the list.
        Ownership is on caller. */
    QMimeData * mimeData(const QList<AbstractObjectItem*>&) const;

    /** Put clipboard data into scene.
        @p gridPos is global and will be retranslated to parent's
        local coords if necessary. */
    void dropMimeData(const QMimeData*, const QPoint& gridPos);

    /** Drop from mouse. Implementation used by AbstractObjectItem
        and the GraphicsScene. If @p parent == 0, root will be used. */
    void dropAction(QGraphicsSceneDragDropEvent*e, Object * parent = 0);

    // ------------------- editing --------------------

    /** Adds an object to object-tree and item-tree.
        @p newObject is completely given away.
        If it can't be added to parent, it will be deleted
        and a message is displayed to the user.
        @p gridPos is local inside parent. */
    void addObject(Object * parent, Object * newObject,
                   const QPoint &gridPos = QPoint(1,1), int insert_index = -1);

    /** Adds an object to object-tree and item-tree.
        The items in @p newObjects are completely given away. If they can't be added to parent,
        they will be deleted and a message is displayed to the user.
        @p gridPos is global and will be retranslated to parent's local coords if necessary. */
    void addObjects(Object * parent, const QList<Object*> newObjects,
                    const QPoint &gridPos = QPoint(1,1), int insert_index = -1);

    /** Deletes the object.
        The object MUST NOT be root! */
    void deleteObject(Object * o);

    void deleteObjects(const QList<AbstractObjectItem*> o);

    //void addModulator(MO::Parameter *, const QString& idName);

private slots:

    void onChanged_();

    // ---------------- signals from editor -----------

    void onObjectAdded_(MO::Object *);
    void onObjectsAdded_(const QList<MO::Object*>&);
    void onObjectDeleted_(const MO::Object *);
    void onObjectsDeleted_(const QList<MO::Object*>&);
    void onObjectMoved_(MO::Object*, MO::Object * oldParent);
    void onObjectNameChanged_(MO::Object *);
    void onObjectColorChanged_(MO::Object *);
    void onObjectChanged_(MO::Object *);
    void onModulatorAdded_(MO::Modulator *);
    void onModulatorDeleted_();
    void onConnectionsChanged_();
    void onParameterVisibilityChanged_(MO::Parameter*);
    void onParameterChanged_(MO::Parameter*);

protected:

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;
    virtual void dropEvent(QGraphicsSceneDragDropEvent*) Q_DECL_OVERRIDE;


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
