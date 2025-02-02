/** @file tree.h

    @brief Basic templated c++ style tree node

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.10.2014</p>
*/

#ifndef MOSRC_GRAPH_TREE_H
#define MOSRC_GRAPH_TREE_H

#if !defined(NDEBUG) && !defined(MO_GRAPH_DEBUG)
#   define MO_GRAPH_DEBUG
#endif


#include <vector>
#include <functional>
#include <queue>

#include <QString>
#include <QList>

//#ifdef MO_GRAPH_DEBUG
#   include <iostream>
//#endif

#include "io/error.h"


namespace MO {


/** Traits class for objects in TreeNode.
    Not exactly a typical traits class,
    this is rather passed as template argument of TreeNode*/
struct TreeNodeTraits
{
    /** This is called when a node is created */
    template <class Node>
    static void creator(Node * node) { (void)node; }
    /** Called in destructor of node */
    template <class Node>
    static void destructor(Node * node) { (void)node; }

    /** Returns a string representation of each node */
    template <class Node>
    static QString toString(const Node * node)
        { return QString("Node(0x%1)").arg((size_t)node, 0, 16); }
};


template <class T, class Traits = TreeNodeTraits>
class TreeNode
{

public:

    // ---------- types --------------

    enum Order
    {
        O_DepthFirst,
        O_BreathFirst
    };

    // -------------- ctor -----------

    TreeNode(T object);
    ~TreeNode();

    // ------- getter --------

    /** Associated object */
    T object() const { return p_obj_; }

    /** Parent node */
    TreeNode * parent() const { return p_parent_; }

    bool isRoot() const { return !p_parent_; }

    /** Number of direct children */
    size_t numChildren() const { return p_child_.size(); }

    /** Read access to std container with children nodes. */
    const std::vector<TreeNode*> & children() const { return p_child_; }

    /** Access to xth children.
        @note No range checking */
    TreeNode * children(size_t index) const { return p_child_[index]; }

    /** Access to children node for associated object.
        Returns 0 if not found. */
    TreeNode * children(T object) const;

    /** Index of children node, -1 or index */
    int indexOf(TreeNode * child) const;

    /** Index of children node, -1 or index */
    int indexOf(const TreeNode * child) const;

    /** Index of children node with associated object, -1 or index */
    int indexOf(T child) const;

    bool hasChild(T child) const { return indexOf(child) >= 0; }
    bool hasChild(TreeNode * child) const { return indexOf(child) >= 0; }

    /** Returns the index of this node in the parents children list.
        If there is no parent, 0 is returned. */
    int indexInParent() const;

    // ------ iterative getter ------

    /** Will return the root of the hierarchy which can be
        the node itself.
        This will never return NULL. */
    TreeNode * root();
    const TreeNode * root() const;

    /** Returns the level below root.
        0 for root itself */
    int countLevel() const;

    /** Creates a copy of this node and it's subtree. */
    TreeNode * copy() const;

    /** Returns the node for the object if it's in the hierarchy, including
        this node itself, or NULL */
    TreeNode * find(T object, bool recursive);
    const TreeNode * find(T object, bool recursive) const;

    /** Returns the node for the object if it's a parent of this node, or NULL */
    TreeNode * findParent(T object) const;

    /** Returns true if @p node is a parent of this node */
    bool hasParent(TreeNode * node) const;

    /** Linearizes the objects into @p objects in the given order to the given level.
        If @p max_level < 0, all levels will be considered, otherwise @p max_level states
        the number of levels to consider, e.g. 0 for none, 1 for the node itself,
        2 for all direct children, etc... */
    void makeLinear(QList<T>& objects, int max_level = -1, Order order = O_DepthFirst) const;
    /** Linearizes all objects that are dynamic_castable to U */
    template <class U>
    void makeLinear(QList<U>& objects, int max_level = -1, Order order = O_DepthFirst) const;



    // ------- getter with selector lambda -------

    /** Creates a copy of this node and it's subtree if @p selector returns true
        for the contained objects.
        @note The node for which this function is called will <b>always</b> be the
        root of the returned tree, regardless of the selector. */
    TreeNode * copy(std::function<bool(const T)> selector) const;

    /** Returns the node for the first object for which @p selector returns true,
        including this node itself, or NULL */
    TreeNode * find(bool recursive, std::function<bool(const T)> selector);
    const TreeNode * find(bool recursive, std::function<bool(const T)> selector) const;

    /** Puts all objects of the hierarchy depth-first into @p objects, if the @p selector
        function returns true.
        Same as makeLiner(objects, selector, O_DepthFirst, recursive ? -1 : 1) */
    void find(QList<T>& objects, bool recursive, std::function<bool(const T)> selector) const
        { makeLinear(objects, selector, recursive ? -1 : 1, O_DepthFirst); }

    /** Returns the parent node for which @p selector returns true, or NULL */
    TreeNode * findParent(std::function<bool(const T)> selector) const;

    /** Linearizes the objects into @p objects in the given order to the given level,
        if @p selector returns true for the objects.
        If @p max_level < 0, all levels will be considered, otherwise @p max_level states
        the number of levels to consider, e.g. 0 for none, 1 for the node itself,
        2 for all direct children, etc... */
    void makeLinear(QList<T>& objects, std::function<bool(const T)> selector,
                    int max_level = -1, Order order = O_DepthFirst) const;
    /** Linearizes all objects that are dynamic_cast'able to U */
    template <class U>
    void makeLinear(QList<U>& objects, std::function<bool(const U)> selector,
                    int max_level = -1, Order order = O_DepthFirst) const;

    // ------- setter --------

    /** Adds the node as children.
        Adding the same node twice leads to undefined behaviour.
        The added node is returned. */
    TreeNode * append(TreeNode * node);

    /** Inserts the node at the given index, -1 for append.
        Adding the same node twice leads to undefined behaviour.
        The inserted node is returned. */
    TreeNode * insert(int index, TreeNode * node);

    /** Adds a new children node with associated object. */
    TreeNode * append(T object);

    /** Inserts a new children node with associated object at the given index,
        -1 for append */
    TreeNode * insert(int index, T object);

    /** Exchanges the two nodes below this node.
        @note No range checking. */
    void swapChildren(size_t from, size_t to);

    /** Destroys the children node with the associated object,
        returns true if found and removed */
    bool remove(T object);

    /** Destroys all the children nodes with the associated object,
        returns true if found and removed */
    bool removeAll(T object);

    /** Destroys and destroys the children node if it exists,
        returns true if removed */
    bool remove(TreeNode * node);

    /** Destroys the xth node.
        @note No range checking */
    void remove(size_t index);

    /** Removes this node from the parent's children list. */
    void detachFromParent();

    /** Takes the children node out of the children list.
        @note No range checking */
    TreeNode * takeChildren(size_t index);

    // ---------- iterative setter ------------

    /** Executes @p func once for each node */
    void forEachNode(std::function<void(TreeNode*)> func);

    // ---------------- debug -----------------

    QString toString() const { return Traits::toString(this); }

//#ifdef MO_GRAPH_DEBUG
    std::ostream& dumpTree(std::ostream& out, const std::string& prepend = "") const;
//#endif

private:

    // disable copy
    TreeNode(const TreeNode&);
    void operator=(const TreeNode&);

    void p_install_node_(TreeNode*);
    TreeNode * p_copy_impl_(TreeNode * inserter,
                            std::function<bool(const T)> selector) const;

    TreeNode * p_parent_;
    std::vector<TreeNode*> p_child_;
    T p_obj_;
};







// #################################### templ impl ##########################################

template <class T, class Traits>
inline TreeNode<T, Traits>::TreeNode(T object)
    : p_parent_(0), p_obj_(object)
{
    Traits::creator(this);
}

template <class T, class Traits>
inline TreeNode<T, Traits>::~TreeNode()
{
    for (auto c : p_child_) delete c;
    Traits::destructor(this);
}



template <class T, class Traits>
inline int TreeNode<T, Traits>::indexOf(TreeNode * child) const
{
    for (size_t i=0; i!=p_child_.size(); ++i)
        if (child == p_child_[i]) return i;
    return -1;
}

template <class T, class Traits>
inline int TreeNode<T, Traits>::indexOf(const TreeNode * child) const
{
    for (size_t i=0; i!=p_child_.size(); ++i)
        if (child == p_child_[i]) return i;
    return -1;
}

template <class T, class Traits>
inline int TreeNode<T, Traits>::indexOf(T child) const
{
    for (size_t i=0; i!=p_child_.size(); ++i)
        if (child == p_child_[i]->object()) return i;
    return -1;
}

template <class T, class Traits>
inline int TreeNode<T, Traits>::indexInParent() const
{
    return parent() ? parent()->indexOf(this) : 0;
}


template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::children(T object) const
{
    const int i = indexOf(object);
    return i < 0 ? 0 : children(i);
}




// ----------------- iterative ----------------

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::root()
{
    TreeNode * p = parent();
    if (!p)
        return this;
    while (p && p->parent())
        p = p->parent();
    return p;
}

template <class T, class Traits>
inline const TreeNode<T, Traits> * TreeNode<T, Traits>::root() const
{
    TreeNode * p = parent();
    if (!p)
        return this;
    while (p && p->parent())
        p = p->parent();
    return p;
}

template <class T, class Traits>
inline int TreeNode<T, Traits>::countLevel() const
{
    int level = 0;
    TreeNode * p = parent();
    while (p)
    {
        p = p->parent();
        ++level;
    }
    return level;
}

// ------------------------- copy --------------------------------

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::copy() const
{
    auto root = new TreeNode(p_obj_);

    for (auto c : p_child_)
        root->append(c->copy());

    return root;
}


template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::p_copy_impl_(
            TreeNode<T, Traits> * inserter,
            std::function<bool(const T)> selector) const
{
    if (selector(object()))
        inserter = inserter->append(new TreeNode(object()));

    for (auto c : children())
        c->p_copy_impl_(inserter, selector);

    return inserter;
}


template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::copy(std::function<bool(const T)> selector) const
{
    TreeNode * root = new TreeNode(object());

    for (auto c : children())
        c->p_copy_impl_(root, selector);

    return root;
}

// ---------------------------- linear ------------------------------

template <class T, class Traits>
template <class U>
void TreeNode<T, Traits>::makeLinear(QList<U>& objects, std::function<bool(const U)> selector,
                             int max_level, Order order) const
{
    if (max_level == 0)
        return;

    if (order == O_DepthFirst)
    {
        // add self
        if (auto u = dynamic_cast<U>(object()))
            if (selector(u))
                objects.append(u);

        if (max_level == 1)
            return;

        for (auto c : children())
            c->makeLinear(objects, selector, max_level < 0 ? -1 : (max_level - 1), order);
    }

    // O_BreathFirst
    else
    {
        std::queue<const TreeNode*> stack;
        std::queue<int> levels;
        stack.push(this);
        levels.push(1);

        while (!stack.empty())
        {
            // take off stack
            const TreeNode * n = stack.front();
            stack.pop();

            // and level
            int level = levels.front();
            levels.pop();
            if (max_level >= 0 && level > max_level)
                break;

            // add to output list
            if (auto u = dynamic_cast<U>(n->object()))
                if (selector(u))
                    objects.append(u);

            // push children
            //   and their levels
            ++level;
            for (auto c : n->children())
            {
                stack.push(c);
                levels.push(level);
            }
        }
    }
}

template <class T, class Traits>
void TreeNode<T, Traits>::makeLinear(QList<T>& objects, std::function<bool(const T)> selector,
                             int max_level, Order order) const
{
    if (max_level == 0)
        return;

    if (order == O_DepthFirst)
    {
        // add self
        if (selector(object()))
            objects.append(object());

        if (max_level == 1)
            return;

        for (auto c : children())
            c->makeLinear(objects, selector, max_level < 0 ? -1 : (max_level - 1), order);
    }

    // O_BreathFirst
    else
    {
        std::queue<const TreeNode*> stack;
        std::queue<int> levels;
        stack.push(this);
        levels.push(1);

        while (!stack.empty())
        {
            // take off stack
            const TreeNode * n = stack.front();
            stack.pop();

            // and level
            int level = levels.front();
            levels.pop();
            if (max_level >= 0 && level > max_level)
                break;

            // add to output list
            if (selector(n->object()))
                objects.append(n->object());

            // push children
            //   and their levels
            ++level;
            for (auto c : n->children())
            {
                stack.push(c);
                levels.push(level);
            }
        }
    }
}


template <class T, class Traits>
template <class U>
void TreeNode<T, Traits>::makeLinear(QList<U>& objects, int max_level, Order order) const
{
    if (max_level == 0)
        return;

    if (order == O_DepthFirst)
    {
        // add self
        if (auto u = dynamic_cast<U>(object()))
            objects.append(u);

        if (max_level == 1)
            return;

        for (auto c : children())
            c->makeLinear(objects, max_level < 0 ? -1 : (max_level - 1), order);
    }

    // O_BreathFirst
    else
    {
        std::queue<const TreeNode*> stack;
        std::queue<int> levels;
        stack.push(this);
        levels.push(1);

        while (!stack.empty())
        {
            // take off stack
            const TreeNode * n = stack.front();
            stack.pop();

            // and level
            int level = levels.front();
            levels.pop();
            if (max_level >= 0 && level > max_level)
                break;

            // add to output list
            if (auto u = dynamic_cast<U>(n->object()))
                objects.append(u);

            // push children
            //   and their levels
            ++level;
            for (auto c : n->children())
            {
                stack.push(c);
                levels.push(level);
            }
        }
    }
}

template <class T, class Traits>
void TreeNode<T, Traits>::makeLinear(QList<T>& objects, int max_level, Order order) const
{
    if (max_level == 0)
        return;

    if (order == O_DepthFirst)
    {
        // add self
        objects.append(object());

        if (max_level == 1)
            return;

        for (auto c : children())
            c->makeLinear(objects, max_level < 0 ? -1 : (max_level - 1), order);
    }

    // O_BreathFirst
    else
    {
        std::queue<const TreeNode*> stack;
        std::queue<int> levels;
        stack.push(this);
        levels.push(1);

        while (!stack.empty())
        {
            // take off stack
            const TreeNode * n = stack.front();
            stack.pop();

            // and level
            int level = levels.front();
            levels.pop();
            if (max_level >= 0 && level > max_level)
                break;

            // add to output list
            objects.append(n->object());

            // push children
            //   and their levels
            ++level;
            for (auto c : n->children())
            {
                stack.push(c);
                levels.push(level);
            }
        }
    }
}



// ---------------------------- find --------------------------------

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::findParent(T o) const
{
    return parent() ?
                parent()->object() == o ? parent()
                                        : parent()->findParent(o)
                    : 0;
}

template <class T, class Traits>
inline bool TreeNode<T, Traits>::hasParent(TreeNode * node) const
{
    return parent() ?
                parent() == node ? true
                                 : parent()->hasParent(node)
                    : false;
}

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::findParent(std::function<bool(const T)> selector) const
{
    return parent() ?
                selector(parent()->object()) ? parent()
                                             : parent()->findParent(selector)
                    : 0;
}

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::find(T o, bool recursive)
{
    if (object() == o)
        return this;

    if (!recursive)
    {
        for (auto c : p_child_)
            if (c->object() == o)
                return c;
    }
    else
    for (auto c : p_child_)
    {
        if (c->object() == o)
            return c;
        if (auto n = c->find(o, true))
            return n;
    }

    return 0;
}

template <class T, class Traits>
inline const TreeNode<T, Traits> * TreeNode<T, Traits>::find(T o, bool recursive) const
{
    if (object() == o)
        return this;

    if (!recursive)
    {
        for (auto c : p_child_)
            if (c->object() == o)
                return c;
    }
    else
    for (auto c : p_child_)
    {
        if (c->object() == o)
            return c;
        if (auto n = c->find(o, true))
            return n;
    }

    return 0;
}

template <class T, class Traits>
TreeNode<T, Traits> * TreeNode<T, Traits>::find(bool recursive, std::function<bool(const T)> selector)
{
    if (selector(object()))
        return this;

    if (!recursive)
    {
        for (auto c : children())
            if (selector(c->object()))
                return c;
    }
    else
    {
        for (auto c : children())
            if (auto n = c->find(true, selector))
                return n;
    }

    return 0;
}

template <class T, class Traits>
const TreeNode<T, Traits> * TreeNode<T, Traits>::find(bool recursive, std::function<bool(const T)> selector) const
{
    if (selector(object()))
        return this;

    if (!recursive)
    {
        for (auto c : children())
            if (selector(c->object()))
                return c;
    }
    else
    {
        for (auto c : children())
            if (auto n = c->find(true, selector))
                return n;
    }

    return 0;
}


// --------------------------------- edit ---------------------------------

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::append(T object)
{
    auto node = new TreeNode(object);
    append(node);
    return node;
}

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::append(TreeNode * node)
{
    MO_ASSERT(indexOf(node) < 0, "TreeNode::append() duplicate node " << node);

    p_child_.push_back(node);
    p_install_node_(node);

    return node;
}

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::insert(int index, T object)
{
    auto node = new TreeNode(object);
    insert(index, node);
    return node;
}

template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::insert(int index, TreeNode * node)
{
    MO_ASSERT(indexOf(node) < 0, "TreeNode<T, Traits>::insert() duplicate node " << node);

    if (index < 0 || index >= (int)p_child_.size())
        p_child_.push_back(node);
    else
        p_child_.insert(
                p_child_.begin()
                    + std::min((int)p_child_.size()-1, index)
                , node);

    p_install_node_(node);

    return node;
}

template <class T, class Traits>
inline void TreeNode<T, Traits>::p_install_node_(TreeNode * node)
{
    node->p_parent_ = this;
}

template <class T, class Traits>
inline void TreeNode<T, Traits>::swapChildren(size_t from, size_t to)
{
    if (from == to)
        return;

    std::swap(p_child_[from], p_child_[to]);
}


template <class T, class Traits>
inline bool TreeNode<T, Traits>::remove(T object)
{
    const int i = indexOf(object);
    if (i < 0)
        return false;
    remove(i);
    return true;
}

template <class T, class Traits>
inline bool TreeNode<T, Traits>::removeAll(T object)
{
    int i = indexOf(object);
    if (i < 0)
        return false;
    do
    {
        remove(i);
        i = indexOf(object);
    } while (i >= 0);

    return true;
}

template <class T, class Traits>
inline bool TreeNode<T, Traits>::remove(TreeNode * node)
{
    const int i = indexOf(node);
    if (i < 0)
        return false;
    remove(i);
    return true;
}

template <class T, class Traits>
inline void TreeNode<T, Traits>::remove(size_t index)
{
    MO_ASSERT(index < p_child_.size(), "TreeNode<T, Traits>::remove(" << index << ") out of range");

    auto node = p_child_[index];
    p_child_.erase(p_child_.begin() + index);
    delete node;
}


template <class T, class Traits>
inline TreeNode<T, Traits> * TreeNode<T, Traits>::takeChildren(size_t index)
{
    MO_ASSERT(index < p_child_.size(), "TreeNode<T, Traits>::takeChildren(" << index << ") out of range");

    auto node = p_child_[index];
    p_child_.erase(p_child_.begin() + index);
    node->p_parent_ = 0;
    return node;
}

template <class T, class Traits>
inline void TreeNode<T, Traits>::detachFromParent()
{
    if (parent())
    {
        int idx = parent()->indexOf(this);
        MO_ASSERT(idx >= 0, "TreeNode<T, Traits>::detachFromParent() i'm not part of parent's children list !?");

        parent()->takeChildren( idx );
    }
}

template <class T, class Traits>
inline void TreeNode<T, Traits>::forEachNode(std::function<void(TreeNode*)> func)
{
    func(this);
    for (auto c : p_child_)
        c->forEachNode(func);
}


// --------------------------------- debug --------------------------------

//#ifdef MO_GRAPH_DEBUG

template <class T, class Traits>
std::ostream& TreeNode<T, Traits>::dumpTree(std::ostream& out, const std::string& prepend) const
{
    out << prepend << toString().toStdString() << std::endl;

    for (auto i : p_child_)
        i->dumpTree(out, " " + prepend);

    return out;
}

//#endif


} // namespace MO

#endif // MOSRC_GRAPH_TREE_H
