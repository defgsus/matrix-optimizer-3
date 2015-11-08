#include <QColor>
#include <QBrush>

#include "csgtreemodel.h"
#include "math/csgbase.h"
#include "io/error.h"

namespace MO {



CsgTreeModel::CsgTreeModel(QObject *parent)
    : QAbstractItemModel        (parent)
    , rootObject_               (0)
{
    headerNames_
            << "name";
}

void CsgTreeModel::setRootObject(CsgRoot* rootObject)
{
    beginResetModel();

    rootObject_ = rootObject;

    endResetModel();
}

CsgBase * CsgTreeModel::nodeForIndex(const QModelIndex& idx) const
{
    if (idx.isValid())
        return static_cast<CsgBase*>( idx.internalPointer() );
    return rootObject_;
}

QModelIndex CsgTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (!rootObject_ || row < 0 || column < 0 || column >= headerNames_.size()
            //|| (parent.isValid() && column != 0)
            )
        return QModelIndex();

    auto pnode = nodeForIndex(parent);

    if (pnode && row < (int)pnode->numChildren())
    {
        return createIndex(row, column, pnode->children()[row]);
    }

    return QModelIndex();
}

QModelIndex CsgTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !rootObject_)
        return QModelIndex();

    if (auto node = nodeForIndex(child))
    {
        // find parent object
        CsgBase * parent = node->parent();
        if (!parent || parent == rootObject_)
            return QModelIndex();

        // get grandparent object
        if (CsgBase * pparent = parent->parent())
        {
            // find index of child
            const int row = pparent->children().indexOf(parent);
            if (row >= 0)
                return createIndex(row, 0, (void*)parent);
        }
    }

    return QModelIndex();
}

int CsgTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!rootObject_)
        return 0;

    if (auto node = nodeForIndex(parent))
        return node->numChildren();

    return 0;
}


int CsgTreeModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() && parent.column() != 0)? 0 : headerNames_.size();
}


QVariant CsgTreeModel::data(const QModelIndex &index, int role) const
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

bool CsgTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!rootObject_ || !index.isValid() || index.column() < 0
            || index.column() >= headerNames_.size())
        return false;

    CsgBase * node = nodeForIndex(index);
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

Qt::ItemFlags CsgTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    if (rootObject_ && index.isValid())
    {
        flag |= Qt::ItemIsSelectable | Qt::ItemIsEnabled
                | Qt::ItemIsEditable;
    }
    return flag;
}

QVariant CsgTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
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
