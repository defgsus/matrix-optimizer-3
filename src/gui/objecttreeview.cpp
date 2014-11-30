/** @file objecttreeeditor.cpp

    @brief QTreeView suitable for MO::ObjectTreeModel

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MO_DISABLE_TREE


#include <QDebug>
#include <QAction>
#include <QMouseEvent>
#include <QMenu>
#include <QCursor>
#include <QClipboard>
#include <QMimeData>
#include <QMessageBox>

#include "objecttreeview.h"
#include "model/objecttreemodel.h"
#include "model/objecttreemimedata.h"
#include "model/objecttreesortproxy.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/model3d.h"
#include "object/objectfactory.h"
#include "object/param/parameterfloat.h"
#include "io/application.h"
#include "io/error.h"
#include "io/log.h"
#include "io/files.h"
#include "geometrydialog.h"
#include "painter/objecttreeviewoverpaint.h"
#include "util/scenesettings.h"

namespace MO {
namespace GUI {


ObjectTreeView::ObjectTreeView(QWidget *parent) :
    QTreeView   (parent),
    filter_     (new ObjectTreeSortProxy(this)),
    scene_      (0),
    lastSelectedObject_(0),
    overpaint_  (new ObjectTreeViewOverpaint(this)),
    sceneSettings_(0),
    sendExpanded_(true)
{
    setStatusTip(tr("Scene tree: right-click to open context menu"));

#ifdef MO_DISABLE_OBJECT_TREE_DRAG
    setDragDropMode(NoDragDrop);
#else
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);
#endif

    setHeaderHidden(true);
    setRootIsDecorated(true);

    connect(this, SIGNAL(expanded(QModelIndex)),
            this, SLOT(onExpanded_(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(onCollapsed_(QModelIndex)));

    // default actions
    createDefaultActions_();

    createEditActions_();
}


QModelIndex ObjectTreeView::getIndexForObject(const Object * obj) const
{
    QModelIndex oidx = omodel_->indexForObject(obj);
    if (!oidx.isValid())
        return QModelIndex();

    QModelIndex idx = filter_->mapFromSource(oidx);
    if (idx.isValid())
        return idx;

    return QModelIndex();
}

QModelIndex ObjectTreeView::getVisibleIndexForObject(Object * obj) const
{
    QModelIndex idx = filter_->mapFromSource(
                omodel_->indexForObject(obj) );

    while (idx.isValid())
    {
        // check if visible
        if (!visualRect(idx).isEmpty())
            return idx;

        // look for parent index
        idx = idx.parent();
    }

    return QModelIndex();
}

Object * ObjectTreeView::objectAbove(const QModelIndex &idx) const
{
    QModelIndex above = indexAbove(idx);
    if (!above.isValid())
        return 0;
    return above.data(ObjectRole).value<Object*>();
}


Object * ObjectTreeView::objectBelow(const QModelIndex &idx) const
{
    QModelIndex below = indexBelow(idx);
    if (!below.isValid())
        return 0;
    return below.data(ObjectRole).value<Object*>();
}

void ObjectTreeView::setObjectModel(ObjectTreeModel *objectModel)
{
    omodel_ = objectModel;

    filter_->setSourceModel(omodel_);
    setModel(filter_);
    sortByColumn(0, Qt::AscendingOrder);

    modelChanged_();

    connect(omodel_, SIGNAL(modelReset()), SLOT(modelChanged_()));
}

void ObjectTreeView::modelChanged_()
{
    scene_ = qobject_cast<Scene*>(omodel_->rootObject());
    overpaint_->updateAll();

    restoreExpansion_();
}

void ObjectTreeView::updateFromModel()
{
    if (!scene_)
        return;

    rowsInserted(rootIndex(), 0, scene_->numChildren());
    //omodel_->setSceneObject(scene_);

    overpaint_->updateAll();
}

void ObjectTreeView::currentChanged(
        const QModelIndex &current, const QModelIndex &/*previous*/)
{
    Object * obj = model()->data(current, ObjectRole).value<Object*>();

    overpaint_->update();

    // create actions with or without object
    createEditActions_(obj);

    // notify mainwindow
    if (obj && obj != lastSelectedObject_)
        emit objectSelected(obj);

    lastSelectedObject_ = obj;
}


void ObjectTreeView::resizeEvent(QResizeEvent *event)
{
    QTreeView::resizeEvent(event);

    QModelIndex idx = currentIndex();
    if (idx.isValid())
        scrollTo(idx);

    overpaint_->resize(size());
    overpaint_->raise();
    overpaint_->updateAll();
}

void ObjectTreeView::mousePressEvent(QMouseEvent * e)
{
    if (!model())
        return;

    QTreeView::mousePressEvent(e);

    if (e->button() == Qt::RightButton)
    {
        QMenu * popup = new QMenu(this);
        connect(popup, SIGNAL(triggered(QAction*)), popup, SLOT(deleteLater()));

        // on empty scene
        if (model()->rowCount() == 0)
            createEditActions_();

        popup->addActions(editActions_);
        if (!popup->isEmpty())
            popup->popup(QCursor::pos());
    }
}

// for sorting the insert-object list
bool sortObjectList_Priority(const Object * o1, const Object * o2)
{
    return Object::objectPriority(o1) > Object::objectPriority(o2);
}


void ObjectTreeView::createDefaultActions_()
{
    createTypeActions_();

    QAction * a;

    a = new QAction(tr("Show/hide objects"), this);
    addAction(a);
    a->setStatusTip(tr("Selects the groups of objects that are visible"));
    a->setMenu(showTypeMenu_);

    addAction(a = new QAction(this));
    a->setSeparator(true);

    a = new QAction(tr("Expand objects"), this);
    addAction(a);
    a->setStatusTip(tr("Expands all real objects, no transformations, tracks or sequences"));
    a->setShortcut(Qt::ALT + Qt::Key_E);
    connect(a, &QAction::triggered, [this](){ expandObjectsByType(Object::TG_REAL_OBJECT); });

    a = new QAction(tr("Expand all"), this);
    addAction(a);
    a->setStatusTip(tr("Expands all objects regardless of their type"));
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_E);
    connect(a, SIGNAL(triggered()), SLOT(expandAll()));

    a = new QAction(tr("Collapse all"), this);
    addAction(a);
    a->setStatusTip(tr("Closes all nodes in the tree"));
    a->setShortcut(Qt::ALT + Qt::Key_C);
    connect(a, SIGNAL(triggered()), SLOT(collapseAll()));

    a = new QAction(tr("RESET MODEL"), this);
    a->setStatusTip(tr("For debugging purposes"));
    addAction(a);
    connect(a, &QAction::triggered, [=]()
    {
        QAbstractItemModel * m(model());
        setModel(0);
        setModel(m);
    });
}

void ObjectTreeView::createTypeActions_()
{
    showTypeMenu_ = new QMenu(tr("Show/hide objects"), this);
    QAction * a;

#define MO__TYPE_ACTION(type__, typename__) \
    showTypeMenu_->addAction(a = new QAction(typename__, this)); \
    a->setData(QVariant((int)(type__))); \
    a->setIcon(ObjectFactory::iconForObject(type__)); \
    a->setCheckable(true); \
    a->setChecked(true);

    MO__TYPE_ACTION(Object::TG_REAL_OBJECT
                    & ~(Object::T_CAMERA | Object::T_MICROPHONE | Object::T_SOUNDSOURCE),
                    tr("Objects"));
    MO__TYPE_ACTION(Object::T_CAMERA, tr("Cameras"));
    MO__TYPE_ACTION(Object::T_MICROPHONE, tr("Microphones"));
    MO__TYPE_ACTION(Object::T_SOUNDSOURCE, tr("Soundsources"));
    MO__TYPE_ACTION(Object::T_TRANSFORMATION, tr("Transformations"));
    MO__TYPE_ACTION(Object::TG_TRACK, tr("Tracks"));
    MO__TYPE_ACTION(Object::TG_SEQUENCE | Object::T_SEQUENCEGROUP, tr("Sequences"));
    MO__TYPE_ACTION(Object::T_CLIP | Object::T_CLIP_CONTAINER, tr("Clips"));
    MO__TYPE_ACTION(Object::T_AUDIO_UNIT, tr("Audio-units"));
    MO__TYPE_ACTION(Object::TG_MODULATOR_OBJECT, tr("Outputs"));

#undef MO__TYPE_ACTION

    connect(showTypeMenu_, &QMenu::triggered, [=]()
    {
        int flags = 0;
        for (auto a : showTypeMenu_->actions())
            if (a->isChecked())
                flags |= a->data().toInt();
        filter_->setObjectTypes(flags);
    });
}


void ObjectTreeView::createEditActions_(Object * obj)
{
    // remove old actions (except default actions)
    for (auto a : editActions_)
        if (!actions().contains(a))
            a->deleteLater();

    editActions_.clear();

    if (!model() || !scene_)
        return;

    //MO_ASSERT(scene_, "No Scene object given for ObjectTreeView with model");

    // object selected?
    if (currentIndex().row()>=0 && obj)
    {
        int numChilds = obj->numChildren(true);

        // title
        QString title(obj->name());
        if (numChilds)
            title += tr(" (%1 children)").arg(numChilds);
        editActions_.addTitle(title, this);

            createClipboardActions_(obj);

        editActions_.addSeparator(this);

            createMoveActions_(obj);

        editActions_.addSeparator(this);

            createEditObjectActions_(obj);

        editActions_.addSeparator(this);

            createNewObjectActions_(obj);

        editActions_.addSeparator(this);

    }
    // no object selected
    else
    {
        createFirstObjectActions_();

        editActions_.addSeparator(this);

    } // no object selected


    // add default actions
    editActions_.append(actions());

    emit editActionsChanged(this, editActions_);
}

QMenu * ObjectTreeView::createObjectsMenu_(Object *parent, bool with_shortcuts)
{
    QList<const Object*> list(ObjectFactory::possibleChildObjects(parent));
    if (list.empty())
        return 0;

    // sort by priority
    qStableSort(list.begin(), list.end(), sortObjectList_Priority);

    QMenu * menu = new QMenu(this);
    int curprio = Object::objectPriority( list.front() );
    for (auto o : list)
    {
        if (curprio != Object::objectPriority(o))
        {
            menu->addSeparator();
            curprio = Object::objectPriority(o);
        }

        QAction * a = new QAction(ObjectFactory::iconForObject(o), o->name(), this);
        a->setData(o->className());
        if (with_shortcuts)
        {
            if (o->className() == "AxisRotate")
                a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_R);
            if (o->className() == "Translate")
                a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_T);
            if (o->className() == "Scale")
                a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_S);
        }
        menu->addAction(a);
    }

    return menu;
}

void ObjectTreeView::createFirstObjectActions_()
{
    QAction * a;

    QMenu * menu = createObjectsMenu_(scene_, true);

    if (menu)
    {
        a = editActions_.addAction(QIcon(":/icon/new.png"), tr("Create object"), this);
        a->setStatusTip(tr("Inserts a new object into the scene"));
        a->setMenu(menu);

        connect(menu, &QMenu::triggered, [this](QAction* a)
        {
            Object * newo = ObjectFactory::createObject(a->data().toString());
            addObject_(rootIndex(), scene_->numChildren(), newo);
        });
    }
}

void ObjectTreeView::createClipboardActions_(Object * obj)
{
    QAction * a;

    // copy
    a = editActions_.addAction(tr("Copy"), this);
    a->setStatusTip(tr("Copies the selected object and all it's children to the clipboard"));
    a->setShortcut(Qt::CTRL + Qt::Key_C);
    connect(a, &QAction::triggered, [=]()
    {
        application->clipboard()->setMimeData(
                    model()->mimeData(QModelIndexList() << currentIndex()));
        // update clipboard actions
        createEditActions_(obj);
    });


    if (obj->canBeDeleted())
    {
        // cut
        a = editActions_.addAction(tr("Cut"), this);
        a->setStatusTip(tr("Moves the selected object and all it's children to the clipboard"));
        a->setShortcut(Qt::CTRL + Qt::Key_X);
        connect(a, &QAction::triggered, [=]()
        {
            application->clipboard()->setMimeData(
                        model()->mimeData(QModelIndexList() << currentIndex()));
            if (deleteObject_(currentIndex()))
                emit objectSelected(0);
            // update clipboard actions
            //createEditActions_(0);
        });

        // delete
        a = editActions_.addAction(tr("Delete"), this);
        a->setStatusTip(tr("Deletes the selected object and all of it's children"));
        a->setShortcut(Qt::CTRL + Qt::Key_Delete);
        a->setIcon(QIcon(":/icon/delete.png"));
        connect(a, &QAction::triggered, [=]()
        {
            if (deleteObject_(currentIndex()))
                emit objectSelected(0);
        });
    }

    Object * parentObj = obj->parentObject();

    // paste
    if (application->clipboard()->mimeData()->formats().contains(
                ObjectTreeMimeData::objectMimeType))
    {
        Object::Type pasteType = static_cast<const ObjectTreeMimeData*>(
                    application->clipboard()->mimeData())->getObjectType();

        if (parentObj)
        {
            // paste before
            a = editActions_.addAction(QIcon(":/icon/above.png"), tr("Paste before object"), this);
            a->setStatusTip(tr("Pastes the objects from the clipboard above the selected object"));
            a->setEnabled(parentObj->canHaveChildren(pasteType));
            connect(a, &QAction::triggered, [=]()
            {
                if (model()->dropMimeData(
                    application->clipboard()->mimeData(), Qt::CopyAction,
                    currentIndex().row(), 0,
                    model()->parent(currentIndex())))
                setFocusIndex(filter_->mapFromSource(omodel_->lastDropIndex()));
            });
        }

        // paste as child
        a = editActions_.addAction(QIcon(":/icon/child.png"), tr("Paste as children"), this);
        a->setStatusTip(tr("Pastes the objects from the clipboard to the end of the "
                           "selected object's children list"));
        a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);
        a->setEnabled(obj->canHaveChildren(pasteType));
        connect(a, &QAction::triggered, [=]()
        {
            if (model()->dropMimeData(
                application->clipboard()->mimeData(), Qt::CopyAction,
                obj->numChildren() /* append */, 0,
                currentIndex()))
            setFocusIndex(filter_->mapFromSource(omodel_->lastDropIndex()));
        });

        if (parentObj)
        {
            // paste after
            a = editActions_.addAction(QIcon(":/icon/below.png"), tr("Paste after object"), this);
            a->setStatusTip(tr("Pastes the objects from the clipboard below the selected object"));
            a->setShortcut(Qt::CTRL + Qt::Key_V);
            a->setEnabled(parentObj->canHaveChildren(pasteType));
            connect(a, &QAction::triggered, [=]()
            {
                if (model()->dropMimeData(
                    application->clipboard()->mimeData(), Qt::CopyAction,
                    currentIndex().row()+1, 0,
                    model()->parent(currentIndex())))
                setFocusIndex(filter_->mapFromSource(omodel_->lastDropIndex()));
            });
        }

    }

    editActions_.addSeparator(this);

    // ---- insert template ----

    a = editActions_.addAction(QIcon(":/icon/new.png"), tr("insert template"), this);
    a->setStatusTip(tr("Inserts a previously saved object from a file"));
    QMenu * sub = new QMenu(this);
    a->setMenu(sub);

        if (parentObj)
        {
            a = sub->addAction(QIcon(":/icon/above.png"), tr("above"));
            a->setStatusTip(tr("Inserts a previously saved object from a file above the current object"));
            connect(a, &QAction::triggered, [=]()
            {
                Object * o = loadObjectTemplate_();
                if (o)
                    addObject_(currentIndex().parent(), parentObj->childObjects().indexOf(obj), o);
            });
        }

        a = sub->addAction(QIcon(":/icon/child.png"), tr("as child"));
        a->setStatusTip(tr("Inserts a previously saved object from a file as child of the current object"));
        connect(a, &QAction::triggered, [=]()
        {
            Object * o = loadObjectTemplate_();
            if (o)
                addObject_(currentIndex(), -1, o);
        });


        if (parentObj)
        {
            a = sub->addAction(QIcon(":/icon/below.png"), tr("below"));
            a->setStatusTip(tr("Inserts a previously saved object from a file below the current object"));
            connect(a, &QAction::triggered, [=]()
            {
                Object * o = loadObjectTemplate_();
                if (o)
                    addObject_(currentIndex().parent(), parentObj->childObjects().indexOf(obj)+1, o);
            });
        }

}


void ObjectTreeView::createEditObjectActions_(Object * obj)
{
    Scene * scene = obj->sceneObject();
    MO_ASSERT(scene, "No scene object for edit-action object");

    QAction * a;

    if (Model3d * m = qobject_cast<Model3d*>(obj))
    {
        a = editActions_.addAction(QIcon(":/icon/obj_3d.png"), tr("Edit model geometry"), this);
        a->setStatusTip(tr("Opens a dialog for editing the model geometry"));
        connect(a, &QAction::triggered, [=]()
        {
            GeometryDialog diag(&m->geometrySettings());

            if (diag.exec() == QDialog::Accepted)
            {
                ScopedObjectChange lock(scene, m);
                m->setGeometrySettings(diag.getGeometrySettings());
            }
        });
    }

    a = editActions_.addAction(tr("Save as template ..."), this);
    a->setStatusTip(tr("Saves the object and it's sub-tree to a file for later reuse"));
    connect(a, &QAction::triggered, [=]()
    {
        storeObjectTemplate_(obj);
    });
}

void ObjectTreeView::createNewObjectActions_(Object * obj)
{
    QAction * a;

    Object * parentObj = obj->parentObject();

    if (parentObj)
    {
        // new sibling above
        QMenu * menu = createObjectsMenu_(parentObj);

        if (menu)
        {
            QModelIndex parentIndex = currentIndex().parent();

            // new sibling above
            a = editActions_.addAction(QIcon(":/icon/new_above.png"), tr("New object above"), this);
            a->setStatusTip(tr("Creates a new object above the selected object"));
            a->setMenu(menu);

            connect(menu, &QMenu::triggered, [this, parentIndex](QAction * a)
            {
                Object * newo = ObjectFactory::createObject(a->data().toString());
                addObject_(parentIndex, currentIndex().row(), newo);
            });
        }
    }

    // new children
    QMenu * menu = createObjectsMenu_(obj);
    if (menu)
    {
        a = editActions_.addAction(QIcon(":/icon/new_child.png"), tr("New child object"), this);
        a->setStatusTip(tr("Creates a new object as children of the selected object"));
        a->setMenu(menu);

        connect(menu, &QMenu::triggered, [this](QAction * a)
        {
            Object * newo = ObjectFactory::createObject(a->data().toString());
            addObject_(currentIndex(), -1, newo);
        });
    }

    if (parentObj)
    {
        // new sibling below
        menu = createObjectsMenu_(parentObj, true);
        if (menu)
        {
            QModelIndex parentIndex = currentIndex().parent();

            // new sibling above
            a = editActions_.addAction(QIcon(":/icon/new_below.png"), tr("New object below"), this);
            a->setStatusTip(tr("Creates a new object below the selected object"));
            a->setMenu(menu);

            connect(menu, &QMenu::triggered, [this, parentIndex](QAction * a)
            {
                Object * newo = ObjectFactory::createObject(a->data().toString());
                addObject_(parentIndex, currentIndex().row() + 1, newo);
            });
        }
    }

}

void ObjectTreeView::createMoveActions_(Object * obj)
{
    if (!obj->parentObject())
        return;

    Object * parent = obj->parentObject();
    QModelIndex curIdx = currentIndex();
    int row = curIdx.row();

    Object * objAbove = objectAbove(curIdx);
    Object * objBelow = objectBelow(curIdx);

    QAction * a;

    // move up
    if (objAbove)
    {
        if (!sortObjectList_Priority(objAbove, obj))
        {
            a = editActions_.addAction(QIcon(":/icon/above.png"), tr("Move up"), this);
            a->setStatusTip(tr("Moves the selected object before the previous object"));
            a->setShortcut(Qt::CTRL + Qt::Key_Up);
            connect(a, &QAction::triggered, [=]()
            {
                omodel_->swapChildren(obj, objAbove);
                setFocusIndex( obj );
                createEditActions_(obj);
            });
        }
    }
    // move down
    if (objBelow)
    {
        if (!sortObjectList_Priority(obj, objBelow))
        {
            a = editActions_.addAction(QIcon(":/icon/below.png"), tr("Move down"), this);
            a->setStatusTip(tr("Moves the selected object below the next object"));
            a->setShortcut(Qt::CTRL + Qt::Key_Down);
            connect(a, &QAction::triggered, [=]()
            {
                omodel_->swapChildren(obj, objBelow);
                setFocusIndex( obj );
                createEditActions_(obj);
            });
        }
    }

    if (obj->canBeDeleted())
    {
        // promote
        if (parent->parentObject() &&
            parent->parentObject()->canHaveChildren(obj->type()))
        {

            a = editActions_.addAction(QIcon(":/icon/left.png"), tr("Promote"), this);
            a->setStatusTip(tr("Moves the selected object one level up in the tree"));
            a->setShortcut(Qt::CTRL + Qt::Key_Left);
            connect(a, &QAction::triggered, [=]()
            {
                setFocusIndex( filter_->mapFromSource( omodel_->promote(obj) ) );
            });
        }

        // demote
        if (row > 0 && objAbove && objAbove->canHaveChildren(obj->type()))
        {

            a = editActions_.addAction(QIcon(":/icon/right.png"), tr("Demote"), this);
            a->setStatusTip(tr("Moves the selected object to the childlist of it's sibling above"));
            a->setShortcut(Qt::CTRL + Qt::Key_Right);
            connect(a, &QAction::triggered, [=]()
            {
                setFocusIndex( filter_->mapFromSource( omodel_->demote(obj) ) );
            });
        }
        else if (row == 0 && objBelow && objBelow->canHaveChildren(obj->type()))
        {
            a = editActions_.addAction(QIcon(":/icon/right.png"), tr("Demote"), this);
            a->setStatusTip(tr("Moves the selected object to the childlist of it's sibling below"));
            a->setShortcut(Qt::CTRL + Qt::Key_Right);
            connect(a, &QAction::triggered, [=]()
            {
                setFocusIndex( filter_->mapFromSource( omodel_->demote(obj) ) );
            });
        }
    }
}

void ObjectTreeView::storeObjectTemplate_(Object * obj)
{
    QString fn = IO::Files::getSaveFileName(IO::FT_OBJECT_TEMPLATE, this);
    if (fn.isEmpty())
        return;

    try
    {
        ObjectFactory::saveObject(fn, obj);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("io error"),
                              tr("Could not save the object template\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
}

Object * ObjectTreeView::loadObjectTemplate_()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_OBJECT_TEMPLATE, this);
    if (fn.isEmpty())
        return 0;

    try
    {
        Object * o = ObjectFactory::loadObject(fn);
        return o;
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("io error"),
                              tr("Could not load the object template\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
    return 0;
}

void ObjectTreeView::setFocusIndex(const QModelIndex & idx)
{
//    qDebug() << idx.row() << idx.column() << idx.data();
    scrollTo(idx);
    setCurrentIndex(idx);

    if (Object * obj = model()->data(currentIndex(), ObjectRole).value<Object*>())
    {
        createEditActions_(obj);
        emit objectSelected(obj);
    }
}

void ObjectTreeView::setFocusIndex(const Object *object)
{
    if (!object)
        return;
    // find index in original model
    QModelIndex oidx = omodel_->indexForObject(object);
    if (!oidx.isValid()) return;
    // find filtered index
    QModelIndex idx = filter_->mapFromSource(oidx);

    // maybe filtered out? then try parent
    while (!idx.isValid())
    {
        object = object->parentObject();
        if (!object) return;
        oidx = omodel_->indexForObject(object);
        if (!oidx.isValid()) return;
        idx = filter_->mapFromSource(oidx);
    }

    if (idx == currentIndex()) return;
    setFocusIndex(idx);
}

void ObjectTreeView::setFocusIndex(const Object *parent, int childRow)
{
    if (childRow >= 0 && childRow < parent->childObjects().size())
    {
        QModelIndex idx = omodel_->indexForObject(parent->childObjects()[childRow]);
        setFocusIndex(filter_->mapFromSource(idx));
    }
}


void ObjectTreeView::expandObjectsByType(int typeflags)
{
    if (!model())
        return;

    // NOTE: omodel_->rootIndex() is the scene object
    // but the filter makes it the first object below root!
    // So we need to work with omodel indices
    QModelIndex idx = omodel_->rootIndex();

    expandObjectsByType_(idx, typeflags | Object::T_SCENE);
}

void ObjectTreeView::expandObjectsByType_(const QModelIndex & index, int typeflags)
{
    Object * obj = omodel_->objectForIndex(index);

    if (obj && (obj->type() & typeflags))
    {
        QModelIndex idx = filter_->mapFromSource(index);
        if (idx.isValid())
            setExpanded(idx, true);
        // traverse children
        for (int i=0; i<omodel_->rowCount(index); ++i)
        {
            expandObjectsByType_(index.child(i, 0), typeflags);
        }
    }
}


bool ObjectTreeView::addObject_(const QModelIndex &parent, int row, Object *obj)
{
    MO_DEBUG_TREE("ObjectTreeView::addObject_("
             << parent << ", " << row << ", " << obj << ")");

    //Object * parentObject = model()->data(parent, ObjectRole).value<Object*>();
    Object * parentObject = parent.data(ObjectRole).value<Object*>();

    if (!parentObject)
        parentObject = scene_;

    if (parentObject)
    {
        QString err;
        if (!parentObject->isSaveToAdd(obj, err))
        {
            QMessageBox::critical(this, tr("Can't do that"),
                                  tr("Can't add %1 to %2.\n%3")
                                  .arg(obj->name())
                                  .arg(parentObject->name())
                                  .arg(err));
            delete obj;
            return false;
        }

        // insert at end
        if (row < 0)
            row = parentObject->numChildren();
        else
        {
            // translate filtered row into omodel row
            // XXX still crap!!
            Object * o = parent.child(row,0).data(ObjectRole).value<Object*>();
            if (o)
            {
                if (o->parentObject())
                {
                    //MO_DEBUG("o=" << o << " p=" << o->parentObject());
                    int newrow = o->parentObject()->childObjects().indexOf(o);
                    //MO_DEBUG("translated row = " << newrow);
                    if (newrow < row)
                        row = parentObject->numChildren();
                    else
                        row = newrow;
                }
            }
        }

        QModelIndex orgidx = omodel_->indexForObject(parentObject);
        QModelIndex idx =
                omodel_->addObject(orgidx, row, obj);

        setFocusIndex(filter_->mapFromSource(idx));

        return true;
    }
    delete obj;

    MO_ASSERT(false, "modelindex has no assigned object");

    return false;
}


bool ObjectTreeView::deleteObject_(const QModelIndex &index)
{
    MO_DEBUG_TREE("ObjectTreeView::deleteObject_(" << index << ")");

    if (index.isValid())
    {
        Object * object = model()->data(index, ObjectRole).value<Object*>();
        if (object)
        {
            QModelIndex orgidx = omodel_->indexForObject(object);
            return omodel_->deleteObject(orgidx);

            //setFocusIndex(filter_->mapFromSource(idx));
        }
        MO_ASSERT(false, "delete-modelindex has no assigned object");
    }
    MO_ASSERT(false, "can't delete object with invalid index");

    return false;
}

void ObjectTreeView::onExpanded_(const QModelIndex & )
{
    if (!sendExpanded_ || !sceneSettings_)
        return;

//    const Object * o = model()->data(idx, ObjectRole).value<Object*>();
//    if (o)
//        sceneSettings_->setExpanded(o, "tv0", true);
}

void ObjectTreeView::onCollapsed_(const QModelIndex & )
{
    if (!sendExpanded_ || !sceneSettings_)
        return;

//    const Object * o = model()->data(idx, ObjectRole).value<Object*>();
//    if (o)
//        sceneSettings_->setExpanded(o, "tv0", false);
}

void ObjectTreeView::restoreExpansion_()
{
    if (!sceneSettings_)
        return;

    // ommit unnecessary updating of SceneSettings
    sendExpanded_ = false;

    restoreExpansion_(sceneObject());

    sendExpanded_ = true;
}

void ObjectTreeView::restoreExpansion_(const Object * o)
{
    QModelIndex idx = getIndexForObject(o);

    //MO_DEBUG("expand " << o << " " << idx);

    if (!o)
        return;

    // expand/collapse this object
    if (idx.isValid())
    {
        bool exp = isExpanded(idx),
             exp2 = sceneSettings_->getExpanded(o, "tv0");
        //MO_DEBUG("expand " << o << " " << idx << " " << (int)exp << " " << (int)exp2);
        if (exp != exp2)
            setExpanded(idx, exp2);
    }

    // traverse childs
    for (auto c : o->childObjects())
    {
        restoreExpansion_(c);
    }
}


} // namespace GUI
} // namespace MO

#endif
