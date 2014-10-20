/** @file treemodel.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.10.2014</p>
*/

#ifndef MOSRC_MODEL_TREEMODEL_H
#define MOSRC_MODEL_TREEMODEL_H

#include <QMetaEnum>
#include <QAbstractItemModel>
#include <QString>
#include <QIcon>
#include <QFont>

#include "graph/tree.h"
#include "object/object_fwd.h"

Q_DECLARE_METATYPE(MO::TreeNode<MO::Object*>*);

namespace MO {

/*
enum
{
    ObjectRole = Qt::UserRole + 1
};
*/

/** Default traits for TreeNode in TreeModel */
template <class T>
struct TreeModelTraits
{
    static QVariant data(TreeNode<T> * node, int role)
    {
        if (role == Qt::DisplayRole)
            return QString("Node(0x%1)").arg((size_t)node, 0, 16);

        return QVariant();
    }

    static bool setData(TreeNode<T> * /*node*/, const QVariant& /*value*/, int /*role*/)
    {
        return false;
    }
};


template <class T>
class TreeModel : public QAbstractItemModel
{
public:
    typedef TreeNode<T> Node;

    enum { NodeRole = Qt::UserRole + 1 };

    explicit TreeModel(Node * root = 0, QObject *parent = 0)
        : QAbstractItemModel(parent), root_(root) { }

    Node * rootNode() const { return root_; }
    void setRootNode(Node * node) { root_ = node; }

    Node * nodeForIndex(const QModelIndex& idx) const
    {
        if (idx.isValid())
            if (auto o = static_cast<Node*>(idx.internalPointer()))
                return o;

        // invalid index means root object
        return root_;
    }

    // --- interface impl. ---

    QModelIndex index(int row, int column, const QModelIndex &parent) const
    {
        if (!root_ || row < 0 || column != 0)
            return QModelIndex();

        Node * pnode = nodeForIndex(parent);

        if (row < pnode->numChildren())
        {
            return createIndex(row, column, (void*)pnode->children(row));

        }

        return QModelIndex();

    }

    QModelIndex parent(const QModelIndex &child) const
    {
        if (!child.isValid() || !root_)
            return QModelIndex();

        if (Node * cnode = nodeForIndex(child))
        {
            // find parent object
            Node * pnode = cnode->parent();
            if (!pnode || pnode == root_)
                return QModelIndex();

            // get grandparent object
            if (Node * gpnode = pnode->parent())
            {
                // find index of grandparent's child
                const int row = gpnode->indexOf(pnode);
                if (row >= 0)
                    return createIndex(row, 0, (void*)pnode);
            }
        }
        return QModelIndex();
    }

    int rowCount(const QModelIndex &parent) const
    {
        if (!root_)
            return 0;

        Node * pnode = nodeForIndex(parent);
        return pnode ? pnode->numChildren() : 0;
    }

    int columnCount(const QModelIndex &parent) const
        { return (parent.isValid() && parent.column() != 0)? 0 : 1; }

    Qt::ItemFlags flags(const QModelIndex &/*index*/) const { return 0; }
    QVariant headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const { return QVariant(); }
    QVariant data(const QModelIndex &index, int role) const
    {
        Node * node = nodeForIndex(index);

        if (!node)
            return QVariant();

        // return pointer to TreeNode
        if (role == NodeRole)
        { QVariant v; v.setValue(node); return v; }

        return TreeModelTraits<T>::data(node, role);

        return QVariant();
    }

    // -- editing --

    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        Node * node = nodeForIndex(index);

        if (!node || role == NodeRole)
            return 0;

        return TreeModelTraits<T>::setData(node, value, role);
    }

private:

    TreeNode<T> * root_;
};




} // namespace MO

#endif // MOSRC_MODEL_TREEMODEL_H
