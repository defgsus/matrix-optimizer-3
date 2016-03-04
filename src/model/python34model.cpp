/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/4/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34_

#include <QColor>
#include <QBrush>

#include "python34model.h"
#include "python/34/py_utils.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {


Python34Model::Python34Model(QObject *parent)
    : QAbstractItemModel        (parent)
    , rootObject_               (0)
{
    headerNames_
            << tr("name") << tr("type");
}

void Python34Model::setRootObject(void* rootObject)
{
    beginResetModel();

    rootObject_ = rootObject;

    endResetModel();
}

void* Python34Model::nodeForIndex(const QModelIndex& idx) const
{
    if (idx.isValid())
        return idx.internalPointer();
    return rootObject_;
}

QList<QModelIndex> Python34Model::getAllIndices() const
{
    QList<QModelIndex> list;
    if (!rootObject_)
        return list;

    addIndices_(rootObject_, list);

    return list;
}

void Python34Model::addIndices_(void *node, QList<QModelIndex> &list) const
{
    for (size_t i=0; i<node->numChildren(); ++i)
    {
        list << createIndex(i, 0, node->children()[i]);
    }

    for (auto n : node->children())
        addIndices_(n, list);
}

QModelIndex Python34Model::index(int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (!rootObject_ || row < 0 || column < 0 || column >= headerNames_.size()
            //|| (parent.isValid() && column != 0)
            )
        return QModelIndex();

    auto pnode = nodeForIndex(parent);

    if (pnode && row < (int)numChildren_(pnode))
    {
        //MO_PRINT("(void*)" << pnode->children()[row]);
        return createIndex(row, column, children_(pnode, row));
    }

    return QModelIndex();
}

QModelIndex Python34Model::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !rootObject_)
        return QModelIndex();

    if (auto node = nodeForIndex(child))
    {
        // find parent object
        void * parent = parent_(node);
        if (!parent || parent == rootObject_)
            return QModelIndex();

        // get grandparent object
        if (void * pparent = parent_(parent))
        {
            // find index of child
            const int row = indexOf_(pparent, parent);
            if (row >= 0)
                return createIndex(row, 0, (void*)parent);
        }
    }

    return QModelIndex();
}

int Python34Model::rowCount(const QModelIndex &parent) const
{
    if (!rootObject_)
        return 0;

    if (auto node = nodeForIndex(parent))
        return node->numChildren();

    return 0;
}


int Python34Model::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() && parent.column() != 0)? 0 : headerNames_.size();
}


QVariant Python34Model::data(const QModelIndex &index, int role) const
{
    if (!rootObject_ || !index.isValid() || index.column() < 0
            || index.column() >= headerNames_.size())
        return QVariant();

    if (auto node = nodeForIndex(index))
    {
        // return edit name
        if (role == Qt::EditRole)
            return node->name();

        // return display name
        if (role == Qt::DisplayRole)
        {
            QString n = node->name();
            if (node->canHaveChildren())
                n += " { }";
            return n;
        }

        // text alignment
        if (role == Qt::TextAlignmentRole)
            return (int)(Qt::AlignLeft | Qt::AlignVCenter);
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

bool Python34Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!rootObject_ || !index.isValid() || index.column() < 0
            || index.column() >= headerNames_.size())
        return false;

    void * node = nodeForIndex(index);
    if (!node)
        return false;

    if (role == Qt::EditRole)
    {
        node->setName(value.toString());
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }

    return false;
}

Qt::ItemFlags Python34Model::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    if (rootObject_ && index.isValid())
    {
        flag |= Qt::ItemIsSelectable | Qt::ItemIsEnabled
                //| Qt::ItemIsEditable
                ;
    }
    return flag;
}

QVariant Python34Model::headerData(int section, Qt::Orientation orientation, int role) const
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

#endif // MO_ENABLE_PYTHON34
