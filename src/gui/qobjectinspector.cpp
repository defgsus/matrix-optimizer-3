/** @file qobjectinspector.cpp

    @brief Display of QObject hierarchy

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include <QTreeView>
#include <QLayout>
#include <QApplication>

#include "qobjectinspector.h"
#include "model/qobjecttreemodel.h"

namespace MO {
namespace GUI {

QObjectInspector::QObjectInspector(QObject * rootObject, QWidget *parent) :
    QMainWindow(parent)
{
    setMinimumSize(600,400);

    model_ = new QObjectTreeModel(rootObject, this);
    //model_->setRootObject(this);//qobject_cast<QObject*>(QApplication::desktop()));

    treeView_ = new QTreeView(this);
    treeView_->setModel(model_);
    treeView_->setColumnWidth(0, 400);
    setCentralWidget(treeView_);
}

} // namespace GUI
} // namespace MO
