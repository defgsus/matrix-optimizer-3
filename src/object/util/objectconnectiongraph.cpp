/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/9/2016</p>
*/

#include "objectconnectiongraph.h"
#include "object/object.h"
#include "graph/directedgraph.h"

namespace MO {

struct ObjectConnectionGraph::Private
{
    Private(ObjectConnectionGraph * p)
        : p         (p)
    { }

    //void getSubGraph(DirectedGraph<Object*>& graph, Object*o);

    ObjectConnectionGraph * p;
    DirectedGraph<Object*> objGraph;
};

ObjectConnectionGraph::ObjectConnectionGraph()
    : p_        (new Private(this))
{

}

ObjectConnectionGraph::~ObjectConnectionGraph()
{
    delete p_;
}

void ObjectConnectionGraph::clear()
{
    p_->objGraph.clear();
}

void ObjectConnectionGraph::addConnection(Object *modulator, Object *goal)
{
    p_->objGraph.addEdge(modulator, goal);
}

bool ObjectConnectionGraph::hasObject(Object* o)
{
    return p_->objGraph.node(o) != 0;
}

bool ObjectConnectionGraph::hasConnection(Object *modulator, Object *goal)
{
    return p_->objGraph.isConnected(modulator, goal);
}
/*
void ObjectConnectionGraph::Private::getSubGraph(DirectedGraph<Object *> &graph, Object *o)
{
    objGraph.node(o);
}
*/
QList<Object*> ObjectConnectionGraph::makeLinear() const
{
    QList<Object*> list;
    p_->objGraph.makeLinear(list);
    return list;
}


} // namespace MO
