/** @file objecttree.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.10.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_OBJECTTREE_H
#define MOSRC_OBJECT_UTIL_OBJECTTREE_H

#include "graph/tree.h"
#include "graph/directedgraph.h"
#include "model/treemodel.h"

namespace MO {

    class Object;

    typedef TreeNode<Object*> ObjectTreeNode;

    /** XXX Returns a copy of the current object tree structure.
        Maybe i switch to hold Objects together with TreeNode<Object*> */
    ObjectTreeNode * getObjectTree(Object* root_object);


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


} // namespace MO


#endif // MOSRC_OBJECT_UTIL_OBJECTTREE_H
