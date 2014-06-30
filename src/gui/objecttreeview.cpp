/** @file objecttreeeditor.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

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

void ObjectTreeView::createEditActions_(Object * obj)
{
    for (auto a : editActions_)
        if (!actions().contains(a))
            a->deleteLater();
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

        // paste
        if (application->clipboard()->mimeData()->formats().contains(
                    ObjectTreeMimeData::mimeType()))
        {
            Object::Type pasteType = static_cast<const ObjectTreeMimeData*>(
                        application->clipboard()->mimeData())->getObjectType();

            const Object * parentObj = obj->parentObject();
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
    }

    // add default actions
    editActions_.append(actions());

    emit editActionsChanged(this, editActions_);
}


} // namespace GUI
} // namespace MO
