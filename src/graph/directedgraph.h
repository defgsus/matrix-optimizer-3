/** @file graph.h

    @brief Templated directed graph structure

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.10.2014</p>
*/

#ifndef MOSRC_GRAPH_DIRECTEDGRAPH_H
#define MOSRC_GRAPH_DIRECTEDGRAPH_H

// for debug/dump functions
#define MO_GRAPH_DEBUG


#include <vector>
#include <map>

#ifdef MO_GRAPH_DEBUG
#   include <iostream>
#endif

#include "io/error.h"

namespace MO {


template <class T>
class DirectedGraph;

/** One node in the graph */
template <class T>
class GraphNode
{
    friend class DirectedGraph<T>;

    GraphNode() { }
    GraphNode(GraphNode&);
public:

    // ---- getter -----

    /** Returns the associated object */
    T object() const { return p_obj_; }

    bool hasInputs() const { return !p_in_.empty(); }
    bool hasOutputs() const { return !p_out_.empty(); }

    bool hasNoInputs() const { return p_in_.empty(); }
    bool hasNoOutputs() const { return p_out_.empty(); }

    size_t numInputs() const { return p_in_.size(); }
    size_t numOutputs() const { return p_out_.size(); }

    GraphNode<T> * input(size_t index) const { return p_in_[index]; }
    GraphNode<T> * output(size_t index) const { return p_out_[index]; }

    bool hasInput(GraphNode<T> * node) const
        { for (auto n : p_in_) if (n == node) return true; return false; }

    bool hasOutput(GraphNode<T> * node) const
        { for (auto n : p_out_) if (n == node) return true; return false; }

    bool hasInput(T object) const
        { for (auto n : p_in_) if (n->object() == object) return true; return false; }

    bool hasOutput(T object) const
        { for (auto n : p_out_) if (n->object() == object) return true; return false; }

    int indexOfInput(GraphNode<T> * node) const
        { for (auto i=0u; i<p_in_.size(); ++i) if (p_in_[i] == node) return i; return -1; }

    int indexOfOutput(GraphNode<T> * node) const
        { for (auto i=0u; i<p_out_.size(); ++i) if (p_out_[i] == node) return i; return -1; }

    int indexOfInput(T object) const
        { for (auto i=0; i<p_in_.size(); ++i) if (p_in_[i]->object() == object) return i; return -1; }

    int indexOfOutput(T object) const
        { for (auto i=0; i<p_out_.size(); ++i) if (p_out_[i]->object() == object) return i; return -1; }

private:

    T p_obj_;
    std::vector<GraphNode<T>*> p_in_, p_out_;
};




/** Directed graph across instances of @p T.
    T is e.g. a pointer-to-object type.
    */
template <class T>
class DirectedGraph
{
public:

    // ---------------- types -------------------------

    typedef GraphNode<T> Node;

    // ------------------ ctor ------------------------

    DirectedGraph() { }
    DirectedGraph(const DirectedGraph<T>& other);
    ~DirectedGraph() { clear(); }

    DirectedGraph<T>& operator = (const DirectedGraph<T>& other);

    // ------------- stl container --------------------

    typedef typename std::map<T, Node*>::const_iterator const_iterator;

    const_iterator begin() const { return p_nodes_.begin(); }
    const_iterator end() const { return p_nodes_.end(); }

    const_iterator rbegin() const { return p_nodes_.rbegin(); }
    const_iterator rend() const { return p_nodes_.rend(); }

    // ------------------ getter ------------------

    /** Returns the Node for the object, or NULL */
    const Node * node(T object) const;
          Node * node(T object);

    bool isConnected(T from, T to) const;

    // ----------------- setter -------------------

    void clear();

    Node * addNode(T object);

    void addEdge(T from, T to);
    void removeEdge(T from, T to);
    void removeEdge(Node * from, Node * to);

    // ------------- transform --------------------

    /** Transforms the graph into a linear list of node objects,
        ordered by input dependency,
        starting with the ones without inputs. */
    template <class Container>
    bool makeLinear(std::insert_iterator<Container> i) const;

    template <class Container>
    bool makeLinear(Container & v) const { return makeLinear(std::inserter(v, v.begin())); }

    // --------------- debug ----------------------

#ifdef MO_GRAPH_DEBUG
    template <typename C>
    std::basic_ostream<C> & dumpEdges(std::basic_ostream<C>& out) const;
#endif

private:

    std::map<T, Node*> p_nodes_;
};




// ################################# temp. impl. ##########################################

template <class T>
DirectedGraph<T>::DirectedGraph(const DirectedGraph<T>& other)
{
    *this = other;
}

template <class T>
DirectedGraph<T>& DirectedGraph<T>::operator = (const DirectedGraph<T>& other)
{
    clear();
    // copy all edges
    for (auto & n : other.p_nodes_)
        for (auto o : n.second->p_out_)
            addEdge(n.first, o->object());

    return *this;
}


template <class T>
void DirectedGraph<T>::clear()
{
    for (auto & i : p_nodes_)
        delete i.second;

    p_nodes_.clear();
}


template <class T>
typename DirectedGraph<T>::Node *
DirectedGraph<T>::node(T object)
{
    auto i = p_nodes_.find(object);
    return i == p_nodes_.end() ? 0 : i->second;
}

template <class T>
const typename DirectedGraph<T>::Node *
DirectedGraph<T>::node(T object) const
{
    auto i = p_nodes_.find(object);
    return i == p_nodes_.end() ? 0 : i->second;
}

template <class T>
typename DirectedGraph<T>::Node *
DirectedGraph<T>::addNode(T object)
{
    if (Node * n = node(object))
        return n;

    // create new
    auto n = new Node();
    n->p_obj_ = object;
    p_nodes_.insert(std::make_pair(object, n));
    return n;
}


template <class T>
bool DirectedGraph<T>::isConnected(T from, T to) const
{
    Node * nf = node(from);
    return nf && nf->hasOutput(to);
}

template <class T>
void DirectedGraph<T>::addEdge(T ofrom, T oto)
{
    Node * from = addNode(ofrom);
    // no duplicates
    if (from->hasOutput(oto))
        return;

    Node * to = addNode(oto);
    // update nodes of each other
    from->p_out_.push_back(to);
    to->p_in_.push_back(from);
}

template <class T>
void DirectedGraph<T>::removeEdge(T ofrom, T oto)
{
    Node * from = node(ofrom);
    if (!from)
        return;

    auto i = from->p_out_.find(oto);
    if (i == from->p_out_.end())
        return;

    // remove in 'from'
    from->p_out_.erase(i);

    // remove in 'to'
    Node * to = node(oto);
    if (!to)
        return;

    i = to->p_in_.find(ofrom);
    if (i != to->p_in_.end())
        to->p_in_.erase(i);
}

template <class T>
void DirectedGraph<T>::removeEdge(Node *from, Node *to)
{
    //std::cout << "remove " << from->object() << " " << to->object() << std::endl;

    int i = from->indexOfOutput(to);
    if (i < 0)
        return;

    // remove in 'from'
    from->p_out_.erase(from->p_out_.begin() + i);

    // remove in 'to'
    i = to->indexOfInput(from);
    MO_ASSERT(i >= 0, "Error in DirectedGraph structure");
    to->p_in_.erase(to->p_in_.begin() + i);
}


// ------------------------------- transform ------------------------------

template <class T>
template <class U>
bool DirectedGraph<T>::makeLinear(std::insert_iterator<U> insertit) const
{
    // copy this graph
    DirectedGraph<T> graph(*this);

    // nodes with no inputs (temporary space)
    std::vector<Node*> beginnings;

    // collect them
    for (auto & n : graph.p_nodes_)
    {
        if (n.second->hasNoInputs()
            && !n.second->hasNoOutputs())
            beginnings.push_back(n.second);
    }

    if (beginnings.empty())
    {
        MO_WARNING("DirectedGraph: no input nodes for makeLinear");
        return false;
    }

    while (!beginnings.empty())
    {
        // remove a node from input-only-modules
        Node * n = beginnings.back();
        beginnings.pop_back();

        // put to sorted list
        insertit = n->object();

        // for each outgoing edge
        while (!n->p_out_.empty())
        {
            // node of output object
            Node * nout = *(n->p_out_.begin());
            MO_ASSERT(nout, "error in DirectedGraph structure");

            // remove this edge
            graph.removeEdge(n, nout);

            // if no other input node, move to beginnings
            if (nout->hasNoInputs())
            {
                beginnings.push_back(nout);
            }
        }
    }

    graph.dumpEdges(std::cout);

    // check if edges left
    for (auto & n : graph.p_nodes_)
    {
        if (n.second->numInputs())
        {
            MO_WARNING("DirectedGraph: detected loop in graph at node " << n.first);
            return false;
        }
    }

    return true;
}




// --------------------------------- debug --------------------------------

#ifdef MO_GRAPH_DEBUG

template <class T>
template <typename C>
std::basic_ostream<C>& DirectedGraph<T>::dumpEdges(std::basic_ostream<C>& out) const
{
    for (auto & i : p_nodes_)
    {
        out << i.first;
        //for (auto j : i.second->p_in_)
        //    out << "\n  " << j << " ->";
        for (auto j : i.second->p_out_)
            out << "\n -> " << j;
        out << std::endl;
    }

    return out;
}

#endif


} // namespace MO

#endif // MOSRC_GRAPH_DIRECTEDGRAPH_H
