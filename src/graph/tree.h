/** @file tree.h

    @brief

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


template <class T>
class Tree;


template <class T>
class TreeNode
{
    //friend class Tree<T>;
public:

    TreeNode(T object) : p_parent_(0), p_obj_(object) { }
    ~TreeNode() { for (auto c : p_child_) delete c; }

    // ------- getter --------

    T object() const { return p_obj_; }

    TreeNode * parent() const { return p_parent_; }

    size_t numChildren() const { return p_child_.size(); }
    TreeNode * children(uint index) const { return p_child_[index]; }

    bool contains(T child) const { return indexOf(child) >= 0; }
    bool contains(TreeNode * child) const { return indexOf(child) >= 0; }

    int indexOf(TreeNode * child) const
        { for (uint i=0; i!=p_child_.size(); ++i)
            if (child == p_child_[i]) return i;
          return -1; }

    int indexOf(T child) const
    { for (uint i=0; i!=p_child_.size(); ++i)
        if (child == p_child_[i]->object()) return i;
      return -1; }

    // ------- setter --------

    TreeNode * add(T object);
    void add(TreeNode * node);
    TreeNode * insert(int index, T object);
    void insert(int index, TreeNode * object);

    bool remove(T object);
    bool remove(TreeNode * node);
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
};




// #################################### templ impl ##########################################

template <class T>
TreeNode<T> * TreeNode<T>::add(T object)
{
    const int i = indexOf(object);
    if (i >= 0)
        return children(i);

    auto node = new TreeNode(object);
    add(node);
    return node;
}

template <class T>
void TreeNode<T>::add(TreeNode * node)
{
    MO_ASSERT(indexOf(node) < 0, "TreeNode<T>::add() duplicate node " << node);

    p_child_.push_back(node);
    p_install_node_(node);
}

template <class T>
TreeNode<T> * TreeNode<T>::insert(int index, T object)
{
    const int i = indexOf(object);
    if (i >= 0)
        return children(i);

    auto node = new TreeNode(object);
    insert(index, node);
    return node;
}

template <class T>
void TreeNode<T>::insert(int index, TreeNode * node)
{
    MO_ASSERT(indexOf(node) < 0, "TreeNode<T>::insert() duplicate node " << node);

    if (index < 0 || p_child_.empty())
        p_child_.push_back(node);
    else
        p_child_.insert(
                p_child_.begin()
                    + std::min((int)p_child_.size()-1, index)
                , node);
}

template <class T>
void TreeNode<T>::p_install_node_(TreeNode * node)
{
    node->p_parent_ = this;
}

template <class T>
bool TreeNode<T>::remove(T object)
{
    const int i = indexOf(object);
    if (i < 0)
        return false;
    remove(i);
    return true;
}

template <class T>
bool TreeNode<T>::remove(TreeNode * node)
{
    const int i = indexOf(object);
    if (i < 0)
        return false;
    remove(i);
    return true;
}

template <class T>
void TreeNode<T>::remove(size_t index)
{
    MO_ASSERT(index < p_child_.size(), "TreeNode<T>::remove(" << index << ") out of range");

    delete p_child_[index];
    p_child_.erase(p_child_.begin() + index);
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
