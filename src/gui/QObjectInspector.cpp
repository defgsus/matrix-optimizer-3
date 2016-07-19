/** @file qobjectinspector.cpp

    @brief Display of QObject hierarchy

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/


#include <QTreeView>
#include <QLayout>
#include <QApplication>
#include <QLayout>
#include <QListWidget>
#include <QMetaProperty>
#include <QDebug>

#include "QObjectInspector.h"
#include "model/QObjectTreeModel.h"
#include "io/Settings.h"

namespace MO {
namespace GUI {

QObjectInspector::QObjectInspector(QObject * rootObject, QWidget *parent) :
    QMainWindow(parent)
{
    setObjectName("_QObjectInspector");


    model_ = new QObjectTreeModel(0, this);
    setRootObject(rootObject);


    setMinimumSize(900,400);
    setCentralWidget(new QWidget(this));

    settings()->restoreGeometry(this);

    auto lh = new QHBoxLayout(centralWidget());

        treeView_ = new QTreeView(this);
        treeView_->setModel(model_);
        treeView_->setColumnWidth(0, 300);
        treeView_->setColumnWidth(1, 200);
        treeView_->setColumnWidth(2, 200);
        connect(treeView_, &QTreeView::clicked, [=](const QModelIndex& idx)
        {
            if (QObject * o = model_->itemForIndex(idx))
                updateList_(o);
        });

        lh->addWidget(treeView_);

        listView_ = new QListWidget(this);
        lh->addWidget(listView_);
}

QObjectInspector::~QObjectInspector()
{
    settings()->storeGeometry(this);
}

void QObjectInspector::setRootObject(QObject *rootObject)
{
    model_->setRootObject(rootObject);

    setWindowTitle(tr("QObject inspector [%1]")
                   .arg(rootObject? rootObject->metaObject()->className() : "-") );
}


void QObjectInspector::updateList_(QObject * o)
{
    listView_->clear();

    for (int i = 0; i < o->metaObject()->propertyCount(); ++i)
    {
        QMetaProperty p = o->metaObject()->property(i);

        QString propStr = p.read(o).toString();
        if (propStr.isEmpty())
        {
            QDebug s(&propStr);
            s << p.read(o);
        }


        listView_->addItem(QString("%1 (%2) = %3")
                           .arg(p.name())
                           .arg(p.typeName())
                           .arg(propStr));
    }
}



} // namespace GUI
} // namespace MO
