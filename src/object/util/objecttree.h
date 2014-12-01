/** @file objecttree.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.10.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTTREE_H
#define MOSRC_OBJECT_UTIL_OBJECTTREE_H

#include "graph/tree.h"
#include "object/object.h"

//#include "graph/directedgraph.h"
//#include "model/treemodel.h"

namespace MO {

    class Object;

    typedef TreeNode<Object*> ObjectTreeNode;

    /** Returns a copy of the current object tree structure.
        XXX There is a lot of work in newobj branch that manages
        the Object tree through ObjectTreeNode but it's not there yet.. */
    ObjectTreeNode * get_object_tree(Object* root_object, bool own = false);

    /** Specialization for Object* in TreeNode */
    template <>
    struct TreeNodeTraits<Object*>
    {
        typedef TreeNode<Object*> Node;

        static void creator(Node * ) { }
        static void destructor(Node * ) { }
        static QString toString(const Node * node)
            { if (node->object()) return node->object()->name();
                else return "Object(null)"; }
    };

#if 0

    namespace Private
    {
        QVariant getModelData(Object*, int role);
        bool setModelData(Object *, const QVariant& data, int role);
    }


    /** specialization for Object* in TreeModel */
    template <>
    struct TreeModelTraits<Object*>
    {
        static QVariant data(TreeNode<Object*> * node, int role)
        {
            return Private::getModelData(node->object(), role);
        }

        static bool setData(TreeNode<Object*> * node, const QVariant& value, int role)
        {
            return Private::setModelData(node->object(), value, role);
        }
    };


    class ObjectTree
    {
    public:

        ObjectTree(Object * root);
        ~ObjectTree();

        void addObject(Object * parent, Object * newChild, int insert_index = -1);
        void deleteObject(Object * object);
        void swapChildren(Object * parent, int from, int to);

    private:

        class Private;
        Private * p_;
    };
#endif


} // namespace MO


#endif // MOSRC_OBJECT_UTIL_OBJECTTREE_H
