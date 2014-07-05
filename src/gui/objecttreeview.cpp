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
#include "object/object.h"
#include "object/scene.h"
#include "object/objectfactory.h"
#include "object/parameterfloat.h"
#include "io/application.h"
#include "io/error.h"


namespace MO {
namespace GUI {


ObjectTreeView::ObjectTreeView(QWidget *parent) :
    QTreeView(parent)
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
    QAction * a;

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

    createEditActions_();

    // --- on item selection ---

    // TODO need keyboard item change as well
    connect(this, &QTreeView::pressed, [this]()
    {
        Object * obj = model()->data(currentIndex(), ObjectRole).value<Object*>();
        createEditActions_(obj);
        if (obj)
        {
            emit objectSelected(obj);
        }
    });
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
        qDebug() << "mp " << editActions_;

        popup->addActions(editActions_);
        if (!popup->isEmpty())
            popup->popup(QCursor::pos());
    }
}

bool sortObjectList_TransformFirst(const Object * o1, const Object * o2)
{
    return o1->isTransformation() && !o2->isTransformation();
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

    if (!model())
        return;

    MO_ASSERT(scene_, "No Scene object given for ObjectTreeView with model");

    QAction * a;

    // object selected?
    if (currentIndex().column() == 0 && obj)
    {
        int numChilds = obj->numChildren(true);

        // title
        QString title(obj->name());
        if (numChilds)
            title += tr(" (%1 children)").arg(numChilds);
        editActions_.append(a = new QAction(title, this));
        QFont font; font.setBold(true);
        a->setFont(font);


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
                    1000000 /* append */, 0,
                    currentIndex());
            });
        }

        editActions_.append(a = new QAction(this));
        a->setSeparator(true);

        // new sibling
        if (parentObj)
        {
            QList<const Object*>
                    plist(ObjectFactory::possibleChildObjects(parentObj));
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
                        /* YYY QModelIndex idx = model->addObject(parentIndex, currentIndex().row()+1, newo);
                        if (!idx.isValid())
                            delete newo;
                        else
                            setFocusIndex(idx);
                            */
                    });
                }
            }
        }

        // new children
        QList<const Object*>
                clist(ObjectFactory::possibleChildObjects(obj));
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
                    /* YYY QModelIndex idx = omodel->addObject(currentIndex(), -1, newo);
                    if (!idx.isValid())
                        delete newo;
                    else
                        setFocusIndex(idx);
                    */
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
                /* YYY QModelIndex idx = omodel->addObject(currentIndex(), -1, seq);
                if (!idx.isValid())
                    delete seq;
                else
                {
                    setFocusIndex(idx);
                    static_cast<ParameterFloat*>(obj)->addModulator(seq->idName());
                }
                */
            });
        }

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
    //QModelIndex sceneIndex = model()->index(0,0,QModelIndex());

    QList<const Object*> plist(ObjectFactory::possibleChildObjects(scene_));

    if (//sceneIndex.isValid() &&
            !plist.isEmpty())
    {
        // make transformations first
        qStableSort(plist.begin(), plist.end(), sortObjectList_TransformFirst);

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
                /* YYY QModelIndex idx = omodel->addObject(sceneIndex, 0, newo);
                if (!idx.isValid())
                    delete newo;
                // XXX hack because of lacking update
                else { expandAll(); setFocusIndex(idx); }
                */
            });
        }
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


} // namespace GUI
} // namespace MO
