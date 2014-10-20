/** @file testdirectedgraph.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.10.2014</p>
*/

#include <QString>
#include <QVector>
#include <vector>

#include "testdirectedgraph.h"

#ifndef MO_GRAPH_DEBUG
#   define MO_GRAPH_DEBUG
#endif

#include "graph/directedgraph.h"

namespace MO {

namespace
{
    // Structure of some object
    struct Thing
    {
        Thing() { }
        Thing(const QString name) : name(name) { }
        QString name;
    };

    template <typename T>
    std::basic_ostream<T>& operator << (std::basic_ostream<T>& out, const Thing * thing)
    {
        if (thing)
            out << "\"" << thing->name.toStdString() << "\"";
        else
            out << "0x0";
        return out;
    }


    int testLinear()
    {
        QVector<Thing> things;

        // create some
        for (int i=0; i<10; ++i)
        {
            things.append(Thing(QString("obj-%1").arg(i+1)));
        }

        // create graph

        DirectedGraph<Thing*> graph;

#if (0)
        // connect
        graph.addEdge(&things[3], &things[0]);
        graph.addEdge(&things[1], &things[2]);
        graph.addEdge(&things[2], &things[3]);

        graph.dumpEdges(std::cout);
#else
        // randomly connect
        for (int i=0; i<10; ++i)
        {
            Thing * from = &things[rand() % things.size()],
                  * to = &things[rand() % things.size()];

            graph.addEdge( from, to );
        }
#endif


        std::vector<Thing*> list;
        bool res = graph.makeLinear(std::inserter(list, list.begin()));

        std::cout << "\nlinear:";
        if (!res)
            std::cout << " (error)";
        for (auto i : list)
            std::cout << " " << i;
        std::cout << std::endl;

        return res ? 1 : 0;
    }

}


TestDirectedGraph::TestDirectedGraph()
{
}





int TestDirectedGraph::run()
{
    int errors = 0;

    errors += testLinear();

    return errors;
}


} // namespace MO
