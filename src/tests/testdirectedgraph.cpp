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
#include "graph/tree.h"
#include "io/log.h"

namespace MO {

//namespace
//{
    // Structure of some object
    struct Thing
    {
        Thing() { }
        Thing(const QString name) : name(name) { }
        QString name;
    };

    // binder to std::out
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




    // ---------------------------- object tree ------------------------------
/** @todo needs to be revised because owning-flag is removed from TreeNode */
#if 0

    class SomeObject;
    typedef TreeNode<SomeObject*> SomeNode;

    class SomeObject
    {
        SomeNode * node_;
        friend class TreeNodeTraits<SomeObject*>;
        static int counter_;
    public:

        QString id;

        SomeObject() : node_(0)
        {
            id = QString::number(counter_++);
            MO_PRINT("obj(" << id << ")");
        }
        ~SomeObject() { MO_PRINT("~obj(" << id << ")"); }

        SomeNode * node() const { return node_; }
    };
    int SomeObject::counter_ = 0;
    /*
    std::ostream& operator << (std::ostream& out, SomeObject* o)
    {
        out << "obj(" << o->id << ")";
        return out;
    }*/

    template <>
    struct TreeNodeTraits<SomeObject*>
    {
        typedef TreeNode<SomeObject*> Node;

        static void creator(Node * node) { if (node->object() && node->isOwning()) node->object()->node_ = node; }
        static void destructor(Node * node) { if (node->isOwning()) delete node->object(); }
        static QString toString(const Node * node)
            { return node->object() ? QString("obj(%1)").arg(node->object()->id)
                                                                     : "null"; }
    };

    typedef TreeNode<SomeObject*> SomeNode;


#define MO__PRINT_LIST(list__) \
    { for (const auto & e__ : list__) \
        std::cout << " " << e__->id; \
        std::cout << std::endl; }

    int testSomeObjectTree()
    {
        auto rootObj = new SomeObject();
        SomeNode root(rootObj);

        SomeNode * insert = &root;
        for (int i=0; i<4; ++i)
        {
            auto node = insert->append(new SomeObject());
            for (int j=0; j<4; ++j)
                node = node->insert(0, new SomeObject());
        }

        root.dumpTree(std::cout);

        MO_PRINT("copy:");
        auto cpy = root.copy(false);
        cpy->dumpTree(std::cout);
        delete cpy;

        MO_PRINT("find:")
        QList<SomeObject*> objs;
        root.find(objs, false, [](const SomeObject*o)
            { return (o->id.toInt() % 2) == 0; } );
        MO__PRINT_LIST(objs);

        MO_PRINT("copy sel:");
        cpy = root.copy(false, [](const SomeObject*o)
        {
            return (o->id.toInt() % 5) != 1;
        });
        cpy->dumpTree(std::cout);
        delete cpy;

        MO_PRINT("make linear depth:");
        objs.clear();
        root.makeLinear(objs, 3, SomeNode::O_DepthFirst);
        MO__PRINT_LIST(objs);
        MO_PRINT("make linear breath:");
        objs.clear();
        root.makeLinear(objs, 3, SomeNode::O_BreathFirst);
        MO__PRINT_LIST(objs);

        auto selector = [](SomeObject * o) { return (o->id.toInt() & 1) != 0; };
        MO_PRINT("make linear depth sel:");
        objs.clear();
        root.makeLinear(objs, selector, SomeNode::O_DepthFirst);
        MO__PRINT_LIST(objs);
        MO_PRINT("make linear breath sel:");
        objs.clear();
        root.makeLinear(objs, selector, SomeNode::O_BreathFirst);
        MO__PRINT_LIST(objs);

        // create a tree with multiple instances
        cpy = new SomeNode(rootObj, false);
        cpy->append(rootObj);
        cpy->append(rootObj)->append(rootObj);
        cpy->append(rootObj)->append(rootObj)->append(rootObj);
        cpy->dumpTree(std::cout);
        delete cpy;

       return 0;
    }

// } namespace
#endif

TestDirectedGraph::TestDirectedGraph()
{
}





int TestDirectedGraph::run()
{
    int errors =
            + testLinear()
        //    + testSomeObjectTree()
            ;

    return errors;
}


} // namespace MO
