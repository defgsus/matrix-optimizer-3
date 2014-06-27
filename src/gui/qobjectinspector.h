/** @file qobjectinspector.h

    @brief Display of QObject hierarchy

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_GUI_QOBJECTINSPECTOR_H
#define MOSRC_GUI_QOBJECTINSPECTOR_H

#include <QMainWindow>

class QTreeView;

namespace MO {
class QObjectTreeModel;
namespace GUI {

class QObjectInspector : public QMainWindow
{
    Q_OBJECT
public:
    explicit QObjectInspector(QObject * rootObject, QWidget *parent = 0);

signals:

public slots:

    void setRootObject(QObject * rootObject);

protected:

    QObjectTreeModel * model_;
    QTreeView * treeView_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_QOBJECTINSPECTOR_H
