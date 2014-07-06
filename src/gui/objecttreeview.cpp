/** @file objecttreeeditor.cpp

    @brief QTreeView suitable for MO::ObjectTreeModel

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QDebug>
#include <QAction>
#include <QMouseEvent>
#include <QMenu>
#include <QCursor>
#include <QClipboard>
#include <QMimeData>

#include "objecttreeview.h"
#include "model/objecttreemodel.h"
#include "model/objecttreemimedata.h"
#include "model/objecttreesortproxy.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/objectfactory.h"
#include "object/parameterfloat.h"
#include "io/application.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {


ObjectTreeView::ObjectTreeView(QWidget *parent) :
    QTreeView   (parent),
    filter_     (new ObjectTreeSortProxy(this))
{
#ifdef MO_DISABLE_OBJECT_TREE_DRAG
    setDragDropMode(NoDragDrop);
#else
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);
#endif

    setHeaderHidden(true);
    setRootIsDecorated(true);

    // default actions
    createDefaultActions_();

    createEditActions_();
}

void ObjectTreeView::setObjectModel(ObjectTreeModel *objectModel)
{
    omodel_ = objectModel;

    filter_->setSourceModel(omodel_);
    setModel(filter_);
    setSortingEnabled(true);

    modelChanged_();

    connect(omodel_, SIGNAL(modelReset()), SLOT(modelChanged_()));
}

void ObjectTreeView::modelChanged_()
{
    scene_ = qobject_cast<Scene*>(omodel_->rootObject());
}


void ObjectTreeView::currentChanged(
        const QModelIndex &current, const QModelIndex &/*previous*/)
{
    Object * obj = model()->data(current, ObjectRole).value<Object*>();

    // create actions with or without object
    createEditActions_(obj);

    // notify mainwindow
    if (obj)
        emit objectSelected(obj);
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

bool sortObjectList_TransformFirst(const Object * o1, const Object * o2)
{
    return o1->isTransformation() && !o2->isTransformation();
}


void ObjectTreeView::createDefaultActions_()
{
    createTypeActions_();

    QAction * a;

    a = new QAction(tr("Show/hide objects"), this);
    addAction(a);
    a->setMenu(showTypeMenu_);

    editActions_.append(a = new QAction(this));
    a->setSeparator(true);

    a = new QAction(tr("Expand objects"), this);
    addAction(a);
    a->setShortcut(Qt::ALT + Qt::Key_E);
    connect(a, SIGNAL(triggered()), SLOT(expandObjectsOnly()));

    a = new QAction(tr("Expand all"), this);
    addAction(a);
    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_E);
    connect(a, SIGNAL(triggered()), SLOT(expandAll()));

    a = new QAction(tr("Collapse all"), this);
    addAction(a);
    a->setShortcut(Qt::ALT + Qt::Key_C);
    connect(a, SIGNAL(triggered()), SLOT(collapseAll()));

    a = new QAction(tr("RESET MODEL"), this);
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
    a->setCheckable(true); \
    a->setChecked(true);

    MO__TYPE_ACTION(Object::T_OBJECT, tr("Objects"));
    MO__TYPE_ACTION(Object::T_CAMERA, tr("Cameras"));
    MO__TYPE_ACTION(Object::T_MICROPHONE, tr("Microphones"));
    MO__TYPE_ACTION(Object::T_SOUNDSOURCE, tr("Soundsources"));
    MO__TYPE_ACTION(Object::T_TRANSFORMATION, tr("Transformations"));
    MO__TYPE_ACTION(Object::TG_PARAMETER, tr("Parameters"));
    MO__TYPE_ACTION(Object::T_TRACK, tr("Tracks"));
    MO__TYPE_ACTION(Object::TG_SEQUENCE | Object::T_SEQUENCEGROUP, tr("Sequences"));

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
    // remove old actions
    for (auto a : editActions_)
        if (!actions().contains(a))
        {
            if (a->menu())
                a->menu()->deleteLater();
            a->deleteLater();
        }

    editActions_.clear();

    if (!model() || !scene_)
        return;

    //MO_ASSERT(scene_, "No Scene object given for ObjectTreeView with model");

    QAction * a;

    // object selected?
    if (currentIndex().row()>=0 && obj)
    {
        int numChilds = obj->numChildren(true);

        // title
        QString title(obj->name());
        if (numChilds)
            title += tr(" (%1 children)").arg(numChilds);
        editActions_.append(a = new QAction(title, this));
        QFont font; font.setBold(true);
        a->setFont(font);

        createClipboardActions_(obj);

        editActions_.append(a = new QAction(this));
        a->setSeparator(true);

        createNewObjectActions_(obj);

        editActions_.append(a = new QAction(this));
        a->setSeparator(true);

    } // object selected?
    else
    {
        createFirstObjectActions_();

        editActions_.append(a = new QAction(this));
        a->setSeparator(true);

    } // no object selected


    // add default actions
    editActions_.append(actions());

    emit editActionsChanged(this, editActions_);
}

void ObjectTreeView::createFirstObjectActions_()
{
    QAction * a;

    // new object below scene
    QModelIndex sceneIndex = model()->index(0,0,QModelIndex());

    QList<const Object*> plist(ObjectFactory::possibleChildObjects(scene_));

    if (//sceneIndex.isValid() &&
            !plist.isEmpty())
    {
        // make transformations first
        qStableSort(plist.begin(), plist.end(), sortObjectList_TransformFirst);

        editActions_.append(a = new QAction(tr("New first object"), this));
        QMenu * menu = new QMenu(this);
        a->setMenu(menu);
        bool addSep = true;
        for (auto o : plist)
        {
            if (addSep && !o->isTransformation())
            {
                menu->addSeparator();
                addSep = false;
            }
            menu->addAction(a = new QAction(ObjectFactory::iconForObject(o), o->name(), this));
            connect(a, &QAction::triggered, [=]()
            {
                Object * newo = ObjectFactory::createObject(o->className());
                addObject_(sceneIndex, scene_->numChildren(), newo);
                    // XXX hack because of lacking update
                    //expandAll();
            });
        }
    }
}

void ObjectTreeView::createClipboardActions_(Object * obj)
{
    QAction * a;

    // copy
    editActions_.append(a = new QAction(tr("Copy"), this));
    connect(a, &QAction::triggered, [=]()
    {
        application->clipboard()->setMimeData(
                    model()->mimeData(QModelIndexList() << currentIndex()));
    });

    if (obj->canBeDeleted())
    {
        // cut
        editActions_.append(a = new QAction(tr("Cut"), this));
        connect(a, &QAction::triggered, [=]()
        {
            application->clipboard()->setMimeData(
                        model()->mimeData(QModelIndexList() << currentIndex()));
            // YYY omodel->deleteObject(currentIndex());
            emit objectSelected(0);
        });

        // delete
        editActions_.append(a = new QAction(tr("Delete"), this));
        connect(a, &QAction::triggered, [=]()
        {
            // YYY omodel->deleteObject(currentIndex());
            emit objectSelected(0);
        });
    }

    Object * parentObj = obj->parentObject();

    // paste
    if (application->clipboard()->mimeData()->formats().contains(
                ObjectTreeMimeData::mimeType()))
    {
        Object::Type pasteType = static_cast<const ObjectTreeMimeData*>(
                    application->clipboard()->mimeData())->getObjectType();

        if (parentObj)
        {
            // paste before
            editActions_.append(a = new QAction(tr("Paste before object"), this));
            a->setEnabled(parentObj->canHaveChildren(pasteType));
            connect(a, &QAction::triggered, [=]()
            {
                model()->dropMimeData(
                    application->clipboard()->mimeData(), Qt::CopyAction,
                    currentIndex().row(), 0,
                    model()->parent(currentIndex()));
            });

            // paste after
            editActions_.append(a = new QAction(tr("Paste after object"), this));
            a->setEnabled(parentObj->canHaveChildren(pasteType));
            connect(a, &QAction::triggered, [=]()
            {
                model()->dropMimeData(
                    application->clipboard()->mimeData(), Qt::CopyAction,
                    currentIndex().row()+1, 0,
                    model()->parent(currentIndex()));
            });
        }

        // paste as child
        editActions_.append(a = new QAction(tr("Paste as children"), this));
        a->setEnabled(obj->canHaveChildren(pasteType));
        connect(a, &QAction::triggered, [=]()
        {
            model()->dropMimeData(
                application->clipboard()->mimeData(), Qt::CopyAction,
                obj->numChildren() /* append */, 0,
                currentIndex());
        });
    }
}

void ObjectTreeView::createNewObjectActions_(Object * obj)
{
    QAction * a;

    Object * parentObj = obj->parentObject();

    // new sibling
    if (parentObj)
    {
        QList<const Object*> plist(ObjectFactory::possibleChildObjects(parentObj));
        if (!plist.isEmpty())
        {
            // make transformations first
            qStableSort(plist.begin(), plist.end(), sortObjectList_TransformFirst);

            QModelIndex parentIndex = model()->parent(currentIndex());

            editActions_.append(a = new QAction(tr("New object"), this));
            QMenu * menu = new QMenu(this);
            a->setMenu(menu);
            bool addSep = true;
            for (auto o : plist)
            {
                if (addSep && !o->isTransformation())
                {
                    menu->addSeparator();
                    addSep = false;
                }
                menu->addAction(a = new QAction(ObjectFactory::iconForObject(o), o->name(), this));
                connect(a, &QAction::triggered, [=]()
                {
                    Object * newo = ObjectFactory::createObject(o->className());
                    addObject_(parentIndex, currentIndex().row()+1, newo);
                });
            }
        }
    }

    // new children
    QList<const Object*> clist(ObjectFactory::possibleChildObjects(obj));
    if (!clist.isEmpty())
    {
        // make transformations first
        qStableSort(clist.begin(), clist.end(), sortObjectList_TransformFirst);

        editActions_.append(a = new QAction(tr("New child object"), this));
        QMenu * menu = new QMenu(this);
        a->setMenu(menu);
        bool addSep = true;
        for (auto o : clist)
        {
            if (addSep && !o->isTransformation())
            {
                menu->addSeparator();
                addSep = false;
            }
            menu->addAction(a = new QAction(ObjectFactory::iconForObject(o), o->name(), this));
            connect(a, &QAction::triggered, [=]()
            {
                Object * newo = ObjectFactory::createObject(o->className());
                addObject_(currentIndex(), -1, newo);
            });
        }
    }

    if (obj->type() == Object::T_PARAMETER_FLOAT)
    {
        editActions_.append(a = new QAction(this));
        a->setSeparator(true);

        editActions_.append(a = new QAction(tr("Add modulation"), this));
        connect(a, &QAction::triggered, [=]()
        {
            Object * seq = ObjectFactory::createObject("SequenceFloat");
            if (addObject_(currentIndex(), -1, seq))
                static_cast<ParameterFloat*>(obj)->addModulator(seq->idName());
        });
    }
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


void ObjectTreeView::expandObjectsOnly()
{
    if (!model())
        return;

    expandObjectOnly_(QModelIndex());
}

void ObjectTreeView::expandObjectOnly_(const QModelIndex & index)
{
    Object * obj = model()->data(index, ObjectRole).value<Object*>();
    if (obj && obj->type() != Object::T_TRANSFORMATION)
    {
        setExpanded(index, true);
        for (int i=0; i<model()->rowCount(index); ++i)
        {
            expandObjectOnly_(model()->index(i, 0, index));
        }
    }
}


bool ObjectTreeView::addObject_(const QModelIndex &parent, int row, Object *obj)
{
    MO_DEBUG_TREE("ObjectTreeView::addObject_(parent("
             << parent.row() << ", " << parent.column() << "), "
             << row << ", " << obj << ")");

    //if (parent.isValid())
    {
        Object * parentObject = model()->data(parent, ObjectRole).value<Object*>();
        if (!parentObject)
            parentObject = scene_;
        if (parentObject)
        {
            if (row < 0)
                row = parentObject->numChildren();

            QModelIndex orgidx = omodel_->indexForObject(parentObject);
            QModelIndex idx = omodel_->addObject(orgidx, row, obj);

            setFocusIndex(filter_->mapFromSource(idx));

            return true;
        }
        delete obj;

        MO_ASSERT(false, "modelindex has no assigned object");
    }
    MO_ASSERT(false, "can't add object to invalid index");

    return false;
}


} // namespace GUI
} // namespace MO
