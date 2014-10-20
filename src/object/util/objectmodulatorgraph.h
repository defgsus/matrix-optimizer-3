/** @file objectmodulatorgraph.h

    @brief Functions for managing modulations between objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.10.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTMODULATORGRAPH_H
#define MOSRC_OBJECT_UTIL_OBJECTMODULATORGRAPH_H

#include "graph/directedgraph.h"
#include "graph/tree.h"

namespace MO {

    class Object;

    typedef DirectedGraph<Object*> ObjectGraph;

    /** Puts all modulation dependencies into the graph object.
        @p graph is not cleared. */
    void getObjectModulatorGraph(ObjectGraph& graph, Object * root);


} // namespace MO

#endif // MOSRC_OBJECT_UTIL_OBJECTMODULATORGRAPH_H
