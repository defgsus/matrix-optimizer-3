/** @file qobjecttreemodel.cpp

    @brief Model to display QObjects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include <QDebug>

#include "qobjecttreemodel.h"

namespace MO {

QObjectTreeModel::QObjectTreeModel(QObject * rootObject, QObject *parent) :
    QAbstractItemModel(parent),
    rootObject_       (rootObject)
{
    headerNames_.append(tr("Class"));
    headerNames_.append(tr("Name"));

    addObject_(rootObject_);
}

void QObjectTreeModel::setRootObject(QObject *rootObject)
{
    beginResetModel();

    rootObject_ = rootObject;

    addObject_(rootObject_);

    endResetModel();
}

QModelIndex QObjectTreeModel::addObject_(QObject * o)
{
    qDebug() << o->metaObject()->className();
    return createIndex(0, 0, (void*)o);
}

QObject * QObjectTreeModel::itemForIndex(const QModelIndex &index) const
{
    //qDebug() << "ifo " << index.row() << index.column();
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

QModelIndex QObjectTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !rootObject_)
        return QModelIndex();

    if (QObject * obj = itemForIndex(child))
    {
        if (obj == rootObject_)
            return QModelIndex();

        QObject * parent = obj->parent();
        if (!parent)
            return QModelIndex();

        const int row = parent->children().indexOf(obj);
        if (row >= 0)
            return createIndex(row, 0, (void*)parent);
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
                default: Q_ASSERT(false);
            }
        }

        if (role == Qt::TextAlignmentRole)
            return (index.column() == 0)?
                        (int)(Qt::AlignLeft | Qt::AlignVCenter)
                    :   (int)(Qt::AlignRight | Qt::AlignVCenter);
    }

    return QVariant();
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
