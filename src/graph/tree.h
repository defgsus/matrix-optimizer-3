/** @file tree.h

    @brief Basic templated c++ style tree node

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.10.2014</p>
*/

#ifndef MOSRC_GRAPH_TREE_H
#define MOSRC_GRAPH_TREE_H

#include <vector>

#ifndef MO_GRAPH_DEBUG
#define MO_GRAPH_DEBUG
#endif

#ifdef MO_GRAPH_DEBUG
#   include <iostream>
#endif

#include "io/error.h"


namespace MO {


//template <class T>
//class Tree;

template <class T>
class TreeNode;

template <class T>
struct TreeNodeTraits
{
    typedef TreeNode<T> Node;

    /** This is called when a node is created */
    static void creator(Node * node) { node=node; }
    /** Called in destructor of node */
    static void destructor(Node * node) { node=node; }
};


template <class T>
class TreeNode
{

public:

    typedef TreeNodeTraits<T> Traits;

    TreeNode(T object, bool own = true)
        : p_parent_(0), p_obj_(object), p_own_(own)
    {
        Traits::creator(this);
    }
    ~TreeNode()
    {
        Traits::destructor(this);
        for (auto c : p_child_) delete c;
    }

    // ------- getter --------

    /** Associated object */
    T object() const { return p_obj_; }

    /** Returns true if the associated object is owned */
    bool isOwning() const { return p_own_; }

    /** Parent node */
    TreeNode * parent() const { return p_parent_; }

    /** Number of direct children */
    size_t numChildren() const { return p_child_.size(); }
    /** Access to xth children.
        Not range checked! */
    TreeNode * children(size_t index) const { return p_child_[index]; }

    /** Access to children node for associated object.
        Returns 0 if not found. */
    TreeNode * children(T object) const;

    /** Index of children node, -1 or index */
    int indexOf(TreeNode * child) const;

    /** Index of children node with associated object, -1 or index */
    int indexOf(T child) const;

    bool hasChild(T child) const { return indexOf(child) >= 0; }
    bool hasChild(TreeNode * child) const { return indexOf(child) >= 0; }

    // ------ more complex getter ------

    /** Will return the root of the hierarchy which can be
        the node itself */
    TreeNode * root() const;

    /** Returns the node for the object if it's in the hierarchy, include
        this node itself, or NULL */
    TreeNode * find(T object) const;

    /** Returns the node for the object if it's a parent of this node, or NULL */
    TreeNode * findParent(T object) const;

    // ------- setter --------

    /** Adds the node as children.
        Adding the same node twice leads to undefined behaviour. */
    void append(TreeNode * node);

    /** Inserts the node at the given index,
        -1 for append */
    void insert(int index, TreeNode * object);

    /** Adds a new children node with associated object.
        Currently multiple adds of the same object are allowed.... */
    TreeNode * append(T object);

    /** Inserts a new children node with associated object at the given index,
        -1 for append */
    TreeNode * insert(int index, T object);

    /** Removes the children node with the associated object,
        returns true if found and removed */
    bool remove(T object);

    /** Removes and destroys the children node if it exists,
        returns true if removed */
    bool remove(TreeNode * node);

    /** Removes and destroys the xth node */
    void remove(size_t index);

    // ---------------- debug -----------------

#ifdef MO_GRAPH_DEBUG
    std::ostream& dumpTree(std::ostream& out, const std::string& prepend = "") const;
#endif

private:

    // disable copy
    TreeNode(const TreeNode&);
    void operator=(const TreeNode&);

    void p_install_node_(TreeNode*);

    TreeNode * p_parent_;
    std::vector<TreeNode*> p_child_;
    T p_obj_;
    bool p_own_;
};







// #################################### templ impl ##########################################

template <class T>
inline int TreeNode<T>::indexOf(TreeNode * child) const
{
    for (size_t i=0; i!=p_child_.size(); ++i)
        if (child == p_child_[i]) return i;
    return -1;
}

template <class T>
inline int TreeNode<T>::indexOf(T child) const
{
    for (size_t i=0; i!=p_child_.size(); ++i)
        if (child == p_child_[i]->object()) return i;
    return -1;
}


template <class T>
inline TreeNode<T> * TreeNode<T>::children(T object) const
{
    const size_t i = indexOf(object);
    return i < 0 ? 0 : children(i);
}

template <class T>
inline TreeNode<T> * TreeNode<T>::append(T object)
{
    //MO_ASSERT(indexOf(node) < 0, "TreeNode<T>::append() duplicate node " << node);

    auto node = new TreeNode(object);
    append(node);
    return node;
}

template <class T>
inline void TreeNode<T>::append(TreeNode * node)
{
    MO_ASSERT(indexOf(node) < 0, "TreeNode<T>::append() duplicate node " << node);

    p_child_.push_back(node);
    p_install_node_(node);
}

template <class T>
inline TreeNode<T> * TreeNode<T>::insert(int index, T object)
{
    const int i = indexOf(object);
    if (i >= 0)
        return children(i);

    auto node = new TreeNode(object);
    insert(index, node);
    return node;
}

template <class T>
inline void TreeNode<T>::insert(int index, TreeNode * node)
{
    MO_ASSERT(indexOf(node) < 0, "TreeNode<T>::insert() duplicate node " << node);

    if (index < 0 || p_child_.empty())
        p_child_.push_back(node);
    else
        p_child_.insert(
                p_child_.begin()
                    + std::min((int)p_child_.size()-1, index)
                , node);

    p_install_node_(node);
}

template <class T>
inline void TreeNode<T>::p_install_node_(TreeNode * node)
{
    node->p_parent_ = this;
}

template <class T>
inline bool TreeNode<T>::remove(T object)
{
    const size_t i = indexOf(object);
    if (i < 0)
        return false;
    remove(i);
    return true;
}

template <class T>
inline bool TreeNode<T>::remove(TreeNode * node)
{
    const size_t i = indexOf(node);
    if (i < 0)
        return false;
    remove(i);
    return true;
}

template <class T>
inline void TreeNode<T>::remove(size_t index)
{
    MO_ASSERT(index < p_child_.size(), "TreeNode<T>::remove(" << index << ") out of range");

    p_child_.erase(p_child_.begin() + index);
    delete p_child_[index];
}



template <class T>
inline TreeNode<T> * TreeNode<T>::root() const
{
    return parent() ? parent()->root() : 0;
}

template <class T>
inline TreeNode<T> * TreeNode<T>::findParent(T o) const
{
    return parent() ?
                parent()->object() == o ? parent()
                                        : parent()->findParent(o)
                    : 0;
}

template <class T>
inline TreeNode<T> * TreeNode<T>::find(T o) const
{
    if (object() == o)
        return this;

    for (auto c : p_child_)
    {
        if (c->object() == o)
            return c;
        if (auto n = c->find(o))
            return n;
    }

    return 0;
}


// --------------------------------- debug --------------------------------

#ifdef MO_GRAPH_DEBUG

template <class T>
std::ostream& TreeNode<T>::dumpTree(std::ostream& out, const std::string& prepend) const
{
    out << prepend << object() << std::endl;

    for (auto i : p_child_)
        i->dumpTree(out, " " + prepend);

    return out;
}

#endif


} // namespace MO

#endif // MOSRC_GRAPH_TREE_H
