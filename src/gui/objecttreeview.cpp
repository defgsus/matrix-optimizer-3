/** @file objecttreeeditor.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

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
    setDragDropMode(DragDrop);
    setDefaultDropAction(Qt::MoveAction);

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
}



void ObjectTreeView::mousePressEvent(QMouseEvent * e)
{
    QTreeView::mousePressEvent(e);

    ObjectTreeModel * omodel = qobject_cast<ObjectTreeModel*>(model());
    Object * obj = omodel->itemForIndex(currentIndex());

    if (e->button() == Qt::RightButton)
    {
        QMenu * popup = new QMenu(this);
        connect(popup, SIGNAL(triggered(QAction*)), popup, SLOT(deleteLater()));

        // click on object?
        if (currentIndex().column() == 0 && obj)
        {
            int numChilds = obj->numChildren(true);
            QAction * a;

            // title
            QString title(obj->name());
            if (numChilds)
                title += tr(" (%1 children)").arg(numChilds);
            popup->addAction(a = new QAction(title, popup));
            QFont font; font.setBold(true);
            a->setFont(font);

            // copy
            popup->addAction(a = new QAction(tr("Copy"), popup));
            connect(a, &QAction::triggered, [=]()
            {
                application->clipboard()->setMimeData(
                            omodel->mimeData(QModelIndexList() << currentIndex()));
            });

            // cut
            popup->addAction(a = new QAction(tr("Cut"), popup));
            connect(a, &QAction::triggered, [=]()
            {
                application->clipboard()->setMimeData(
                            omodel->mimeData(QModelIndexList() << currentIndex()));
                omodel->deleteObject(currentIndex());
            });

            // delete
            popup->addAction(a = new QAction(tr("Delete"), popup));
            connect(a, &QAction::triggered, [=](){ omodel->deleteObject(currentIndex()); });

            // paste
            if (application->clipboard()->mimeData()->formats().contains(
                        ObjectTreeMimeData::mimeType()))
            {
                popup->addAction(a = new QAction(tr("Paste before object"), popup));
                connect(a, &QAction::triggered, [=]()
                {
                    omodel->dropMimeData(
                        application->clipboard()->mimeData(), Qt::CopyAction,
                        currentIndex().row(), 0,
                        omodel->parent(currentIndex()));
                });
                popup->addAction(a = new QAction(tr("Paste as child"), popup));
                connect(a, &QAction::triggered, [=]()
                {
                    omodel->dropMimeData(
                        application->clipboard()->mimeData(), Qt::CopyAction,
                        currentIndex().row(), 0,
                        currentIndex());
                });
            }

            popup->addSeparator();
        }

        popup->addActions(actions());
        popup->popup(QCursor::pos());
    }
}

} // namespace GUI
} // namespace MO
