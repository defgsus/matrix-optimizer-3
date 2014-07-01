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
#include "object/objectfactory.h"
#include "io/application.h"

namespace MO {
namespace GUI {


ObjectTreeView::ObjectTreeView(QWidget *parent) :
    QTreeView(parent)
{
    setDragEnabled(true);
#ifdef MO_DISABLE_OBJECT_TREE_DRAG
    setDragDropMode(NoDragDrop);
#else
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);
#endif

    // default actions
    QAction * a;

    a = new QAction(tr("Expand all"), this);
    addAction(a);
    a->setShortcut(Qt::ALT + Qt::Key_E);
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
        ObjectTreeModel * omodel = qobject_cast<ObjectTreeModel*>(model());
        Object * obj = omodel->itemForIndex(currentIndex());
        createEditActions_(obj);
        if (obj)
        {
            emit objectSelected(obj);
        }
    });
}



void ObjectTreeView::mousePressEvent(QMouseEvent * e)
{
    QTreeView::mousePressEvent(e);

    if (e->button() == Qt::RightButton)
    {
        QMenu * popup = new QMenu(this);
        connect(popup, SIGNAL(triggered(QAction*)), popup, SLOT(deleteLater()));

        popup->addActions(editActions_);

        popup->popup(QCursor::pos());
    }
}

bool sortObjecListLessThan(const Object * o1, const Object * o2)
{
    return o1->isTransformation() && !o2->isTransformation();
}

void ObjectTreeView::createEditActions_(Object * obj)
{
    for (auto a : editActions_)
        if (!actions().contains(a))
        {
            if (a->menu())
                a->menu()->deleteLater();
            a->deleteLater();
        }
    editActions_.clear();

    // click on object?
    if (currentIndex().column() == 0 && obj)
    {
        ObjectTreeModel * omodel = qobject_cast<ObjectTreeModel*>(model());

        int numChilds = obj->numChildren(true);
        QAction * a;

        // title
        QString title(obj->name());
        if (numChilds)
            title += tr(" (%1 children)").arg(numChilds);
        editActions_.append(a = new QAction(title, this));
        QFont font; font.setBold(true);
        a->setFont(font);

        if (!obj->isParameter())
        {
            // copy
            editActions_.append(a = new QAction(tr("Copy"), this));
            connect(a, &QAction::triggered, [=]()
            {
                application->clipboard()->setMimeData(
                            omodel->mimeData(QModelIndexList() << currentIndex()));
            });

            // cut
            editActions_.append(a = new QAction(tr("Cut"), this));
            connect(a, &QAction::triggered, [=]()
            {
                application->clipboard()->setMimeData(
                            omodel->mimeData(QModelIndexList() << currentIndex()));
                omodel->deleteObject(currentIndex());
            });

            // delete
            editActions_.append(a = new QAction(tr("Delete"), this));
            connect(a, &QAction::triggered, [=](){ omodel->deleteObject(currentIndex()); });
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
                    omodel->dropMimeData(
                        application->clipboard()->mimeData(), Qt::CopyAction,
                        currentIndex().row(), 0,
                        omodel->parent(currentIndex()));
                });

                // paste after
                editActions_.append(a = new QAction(tr("Paste after object"), this));
                a->setEnabled(parentObj->canHaveChildren(pasteType));
                connect(a, &QAction::triggered, [=]()
                {
                    omodel->dropMimeData(
                        application->clipboard()->mimeData(), Qt::CopyAction,
                        currentIndex().row()+1, 0,
                        omodel->parent(currentIndex()));
                });
            }

            // paste as child
            editActions_.append(a = new QAction(tr("Paste as children"), this));
            a->setEnabled(obj->canHaveChildren(pasteType));
            connect(a, &QAction::triggered, [=]()
            {
                omodel->dropMimeData(
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
                qStableSort(plist.begin(), plist.end(), sortObjecListLessThan);

                QModelIndex parentIndex = omodel->parent(currentIndex());

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
                        if (!omodel->addObject(parentIndex, currentIndex().row()+1, newo))
                            delete newo;
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
            qStableSort(clist.begin(), clist.end(), sortObjecListLessThan);

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
                    if (!omodel->addObject(currentIndex(), -1, newo))
                        delete newo;
                });
            }
        }


        editActions_.append(a = new QAction(this));
        a->setSeparator(true);
    }

    // add default actions
    editActions_.append(actions());

    emit editActionsChanged(this, editActions_);
}


} // namespace GUI
} // namespace MO
