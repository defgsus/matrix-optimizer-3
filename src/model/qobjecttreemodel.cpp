/** @file qobjecttreemodel.cpp

    @brief Model to display QObjects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include <QDebug>
#include <QColor>
#include <QBrush>

#include "qobjecttreemodel.h"

namespace MO {

QObjectTreeModel::QObjectTreeModel(QObject * rootObject, QObject *parent) :
    QAbstractItemModel(parent),
    rootObject_       (rootObject)
{
    headerNames_
            << "Class"
            << "Name"
            << "info count";
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
    // invalid index means root object (??)
    return rootObject_;
}


QModelIndex QObjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (!rootObject_ || row < 0 || column < 0 || column >= headerNames_.size()
            || (parent.isValid() && column != 0))
        return QModelIndex();

    QObject * obj = itemForIndex(parent);
    //Q_ASSERT(obj);
    if (!obj)
        return QModelIndex();

    if (row < obj->children().size())
    {
        return createIndex(row, column, obj->children()[row]);
    }

    return QModelIndex();
}

QModelIndex QObjectTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid() || !rootObject_)
        return QModelIndex();

    if (QObject * obj = itemForIndex(index))
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
                case 2: return QString::number(obj->metaObject()->classInfoCount());
                default: Q_ASSERT(false);
            }
        }

        // text alignment
        if (role == Qt::TextAlignmentRole)
            return (index.column() == 0)?
                        (int)(Qt::AlignLeft | Qt::AlignVCenter)
                    :   (int)(Qt::AlignRight | Qt::AlignVCenter);

        /*if (role == Qt::BackgroundRole)
        {
            return QBrush(Qt::gray);
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


} // namespace MO
