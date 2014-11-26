/** @file objecttreeeditor.h

    @brief QTreeView suitable for MO::ObjectTreeModel

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MO_DISABLE_TREE


#ifndef MOSRC_GUI_OBJECTTREEVIEW_H
#define MOSRC_GUI_OBJECTTREEVIEW_H

#include <QTreeView>
#include <QList>

#include "tool/actionlist.h"

class QMenu;
namespace MO {
class Object;
class Scene;
class ObjectTreeModel;
class ObjectTreeSortProxy;
namespace GUI {

class ObjectTreeViewOverpaint;
class SceneSettings;

class ObjectTreeView : public QTreeView
{
    Q_OBJECT

    friend class ObjectTreeViewOverpaint;

public:

    explicit ObjectTreeView(QWidget *parent = 0);

    void setObjectModel(ObjectTreeModel * objectModel);

    void setSceneSettings(SceneSettings * s) { sceneSettings_ = s; }

    Scene * sceneObject() const { return scene_; }

    /** Returns the index (into the filter-model) for a given object. */
    QModelIndex getIndexForObject(const Object *) const;

    /** Returns the index (into the filter-model) of a given object.
        If the item is not visible because a parent item is not expanded,
        the next visible parent index will be returned. */
    QModelIndex getVisibleIndexForObject(Object *) const;

    /** Returns the object above idx (filter-model), or NULL */
    Object * objectAbove(const QModelIndex& idx) const;

    /** Returns the object below idx (filter-model), or NULL */
    Object * objectBelow(const QModelIndex& idx) const;

signals:

    /** Sends a list of actions for the edit menu. */
    void editActionsChanged(const QObject * sender, const QList<QAction*>&);

    /** Emitted when an object was selected.
        When a selected object was deleted, @p object will be NULL. */
    void objectSelected(MO::Object * object);

public slots:

    /** Force update of whole model data */
    void updateFromModel();

    void expandObjectsByType(int typeflags);

    void setFocusIndex(const QModelIndex&);
    void setFocusIndex(const Object * object);
    void setFocusIndex(const Object * parent, int childRow);

protected slots:

    void modelChanged_();
    void onExpanded_(const QModelIndex&);
    void onCollapsed_(const QModelIndex&);

protected:

    void setModel(QAbstractItemModel * m) { QTreeView::setModel(m); }

    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

    void createDefaultActions_();
    void createTypeActions_();
    void createEditActions_(Object* = 0);
    void createFirstObjectActions_();
    void createClipboardActions_(Object *);
    void createEditObjectActions_(Object *);
    void createNewObjectActions_(Object *);
    void createMoveActions_(Object *);

    /** Sets the expanded-flags according to SceneSettings */
    void restoreExpansion_();
    void restoreExpansion_(const Object*);

    /** Creates a menu with all possibly child objects for @p parent.
        Each action has the Object::className() in it's data QVariant.
        If no child objects are possible, NULL is returned! */
    QMenu * createObjectsMenu_(Object * parent, bool with_shortcuts = false);

    void expandObjectsByType_(const QModelIndex& indexInOriginalModel, int typeflags);

    /** Open file dialog */
    void storeObjectTemplate_(Object *);
    /** Opens filedialig, returns an object or NULL */
    Object * loadObjectTemplate_();

    /** highlevel do-it-all function for adding objects in the tree. */
    bool addObject_(const QModelIndex& parent, int row, Object * obj);
    bool deleteObject_(const QModelIndex&);

    ActionList editActions_;

    QMenu * showTypeMenu_;

    ObjectTreeModel * omodel_;
    ObjectTreeSortProxy * filter_;
    Scene * scene_;
    Object * lastSelectedObject_;

    ObjectTreeViewOverpaint * overpaint_;

    SceneSettings * sceneSettings_;

    bool sendExpanded_;

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_OBJECTTREEVIEW_H

#endif
