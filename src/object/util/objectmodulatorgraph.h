/** @file objectmodulatorgraph.h

    @brief Functions for managing modulations between objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.10.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTMODULATORGRAPH_H
#define MOSRC_OBJECT_UTIL_OBJECTMODULATORGRAPH_H

#include <map>

#include "graph/directedgraph.h"
#include "graph/tree.h"

namespace MO {

    class Object;

    typedef GraphNode<Object*> ObjectGraphNode;
    typedef DirectedGraph<Object*> ObjectGraph;

    /** Puts all modulation dependencies into the graph object.
        @p graph is not cleared. */
    void get_object_modulator_graph(ObjectGraph& graph, Object * root);


    // XXX belongs into graph/conversion.h or something
    /*
    XXX not really working nor sensible
    template <class T>
    void getModulationTree(TreeNode<T> * root, const DirectedGraph<T>& graph)
    {
        std::vector<T> linear;
        graph.makeLinear(linear);

        // remember treenodes and objects
        std::map<T, TreeNode<T>*> nodes;

        for (auto i = linear.rbegin(); i!=linear.rend(); ++i)
        {
            const GraphNode<T> * n = graph.node(*i);
            MO_ASSERT(n, "error in linearized DirectedGraph");

            // store output nodes at top-level
            if (n->hasInputs() && n->hasNoOutputs())
                nodes.insert(std::make_pair(n->object(), root->add(n->object()) ));

            // if node has outputs it's lower level
            for (auto j = 0; j < n->numOutputs(); ++j)
            {
                auto pit = nodes.find(n->output(j)->object());
                if (pit == nodes.end())
                {
                    MO_WARNING("output node not in tree");
                }
                else
                {
                    nodes.insert(std::make_pair(n->object(), pit->second->add(n->object()) ));
                }
            }
        }
    }
    */

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_OBJECTMODULATORGRAPH_H
