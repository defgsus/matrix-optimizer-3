/** @file objecttreeeditor.h

    @brief QTreeView suitable for MO::ObjectTreeModel

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_GUI_OBJECTTREEVIEW_H
#define MOSRC_GUI_OBJECTTREEVIEW_H

#include <QTreeView>

namespace MO {
namespace GUI {


class ObjectTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit ObjectTreeView(QWidget *parent = 0);

signals:

public slots:

protected:

    void mousePressEvent(QMouseEvent *);

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_OBJECTTREEVIEW_H
