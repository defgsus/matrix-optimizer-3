/** @file object1tree.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.11.2014</p>
*/

#ifndef OBJECT1TREE_H
#define OBJECT1TREE_H

#include "graph/tree.h"
#include "graph/directedgraph.h"
#include "model/treemodel.h"

namespace MO {

    class Object1;

    typedef TreeNode<Object1*> Object1TreeNode;

    namespace Private
    {
        QVariant getModelData(Object1*, int role);
        bool setModelData(Object1*, const QVariant& data, int role);
    }


    /** specialization for Object* in TreeModel */
    template <>
    struct TreeModelTraits<Object1*>
    {
        static QVariant data(TreeNode<Object1*> * node, int role)
        {
            return Private::getModelData(node->object(), role);
        }

        static bool setData(TreeNode<Object1*> * node, const QVariant& value, int role)
        {
            return Private::setModelData(node->object(), value, role);
        }
    };



    class Object1Tree
    {
    public:

        Object1Tree(Object1 * root);
        ~Object1Tree();

        void addObject(Object1 * parent, Object1 * newChild, int insert_index = -1);
        void deleteObject(Object1 * object);
        void swapChildren(Object1 * parent, int from, int to);

    private:

        class Private;
        Private * p_;
    };


} // namespace MO


#endif // OBJECT1TREE_H
