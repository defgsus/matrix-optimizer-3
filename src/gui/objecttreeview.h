/** @file objecttreeeditor.h

    @brief QTreeView suitable for MO::ObjectTreeModel

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_GUI_OBJECTTREEVIEW_H
#define MOSRC_GUI_OBJECTTREEVIEW_H

#include <QTreeView>
#include <QList>

class QMenu;
namespace MO {
class Object;
namespace GUI {


class ObjectTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit ObjectTreeView(QWidget *parent = 0);

signals:

    /** Sends a list of actions for the edit menu. */
    void editActionsChanged(const QObject * sender, const QList<QAction*>&);

    /** Emitted when an object was selected.
        When a selected object was deleted, @p object will be NULL. */
    void objectSelected(MO::Object * object);

public slots:

protected:

    void mousePressEvent(QMouseEvent *);

    void createEditActions_(Object* = 0);

    QList<QAction*> editActions_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_OBJECTTREEVIEW_H
