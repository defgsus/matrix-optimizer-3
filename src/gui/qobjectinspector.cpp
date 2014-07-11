/** @file qobjectinspector.cpp

    @brief Display of QObject hierarchy

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

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
    setMinimumSize(900,400);

    model_ = new QObjectTreeModel(0, this);
    setRootObject(rootObject);

    treeView_ = new QTreeView(this);
    treeView_->setModel(model_);
    treeView_->setColumnWidth(0, 300);
    treeView_->setColumnWidth(1, 200);
    treeView_->setColumnWidth(2, 200);
    setCentralWidget(treeView_);
}

void QObjectInspector::setRootObject(QObject *rootObject)
{
    model_->setRootObject(rootObject);

    setWindowTitle(tr("QObject inspector [%1]")
                   .arg(rootObject? rootObject->metaObject()->className() : "-") );
}


} // namespace GUI
} // namespace MO
