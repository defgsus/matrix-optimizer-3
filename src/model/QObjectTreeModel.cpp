/** @file qobjecttreemodel.cpp

    @brief Model to display QObjects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/


#include <QDebug>
#include <QColor>
#include <QBrush>

// for object properties
#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QLayout>

#include "QObjectTreeModel.h"
#include "io/error.h"

namespace MO {

QObjectTreeModel::QObjectTreeModel(QObject * rootObject, QObject *parent) :
    QAbstractItemModel(parent),
    rootObject_       (rootObject)
{
    headerNames_
            << "class"
            << "object name"
            << "text/title"
            << "rect";
}

void QObjectTreeModel::setRootObject(QObject *rootObject)
{
    beginResetModel();

    rootObject_ = rootObject;

    endResetModel();
}

QObject * QObjectTreeModel::itemForIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (auto o = static_cast<QObject*>(index.internalPointer()))
        {
            return o;
        }
    }
    // invalid index means root object
    return rootObject_;
}


QModelIndex QObjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (!rootObject_ || row < 0 || column < 0 || column >= headerNames_.size()
            //|| (parent.isValid() && column != 0)
            )
        return QModelIndex();

    QObject * obj = itemForIndex(parent);
    MO_ASSERT(obj, "no object for index <"<<parent.row()<<","<<parent.column()<<">");

    if (row < obj->children().size())
    {
        return createIndex(row, column, obj->children()[row]);
    }

    return QModelIndex();
}

QModelIndex QObjectTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !rootObject_)
        return QModelIndex();

    if (QObject * obj = itemForIndex(child))
    {
        // find parent object
        QObject * parent = obj->parent();
        if (!parent || parent == rootObject_)
            return QModelIndex();

        // get grandparent object
        if (QObject * pparent = parent->parent())
        {
            // find index of child
            const int row = pparent->children().indexOf(parent);
            if (row >= 0)
                return createIndex(row, 0, (void*)parent);
        }
    }

    return QModelIndex();
}

int QObjectTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!rootObject_)
        return 0;

    if (QObject * obj = itemForIndex(parent))
        return obj->children().size();

    return 0;
}


int QObjectTreeModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() && parent.column() != 0)? 0 : headerNames_.size();
}


QVariant QObjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!rootObject_ || !index.isValid() || index.column() < 0
            || index.column() >= headerNames_.size())
        return QVariant();

    if (QObject * obj = itemForIndex(index))
    {
        // return text
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            switch (index.column())
            {
                case 0: return obj->metaObject()->className();
                case 1: return obj->objectName();
                case 2: return objectTitle(obj);
                case 3: return objectRect(obj);
                default: MO_LOGIC_ERROR("no DisplayRole defined for column " << index.column());
            }
        }

        // text alignment
        if (role == Qt::TextAlignmentRole)
            return (index.column() == 0)?
                        (int)(Qt::AlignLeft | Qt::AlignVCenter)
                    :   (int)(Qt::AlignRight | Qt::AlignVCenter);
        /*
        if (role == Qt::BackgroundRole)
        {
            if (index.column() == 2)
                return QBrush(Qt::gray);
            if (index.column() == 1)
                return QBrush(Qt::green);
        }*/
    }

    return QVariant();
}

Qt::ItemFlags QObjectTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    if (rootObject_ && index.isValid())
    {
        flag |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return flag;
}

QVariant QObjectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal && section < headerNames_.size())
        {
            return headerNames_[section];
        }
    }

    return QVariant();
}



QString QObjectTreeModel::objectTitle(QObject * o)
{
    if (auto w = qobject_cast<QAction*>(o))
        return w->text();
    if (auto w = qobject_cast<QMenu*>(o))
        return w->title();
    if (auto w = qobject_cast<QLabel*>(o))
        return w->text();

    if (auto w = qobject_cast<QWidget*>(o))
        return w->windowTitle();


    return QString();
}

QString QObjectTreeModel::objectRect(QObject * o)
{
    QRect r;
    if (auto w = qobject_cast<QWidget*>(o))
        r = w->rect();
    else if (auto w = qobject_cast<QLayout*>(o))
        r = w->geometry();
    else
        return QString();

    if (r.x() == 0 && r.y() == 0)
        return QString("%1x%2").arg(r.width()).arg(r.height());
    else
        return QString("%1,%2 %3x%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
}


} // namespace MO
