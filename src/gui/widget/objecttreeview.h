/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_OBJECTTREEVIEW_H
#define MOSRC_GUI_WIDGET_OBJECTTREEVIEW_H

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
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_OBJECTTREEVIEW_H
