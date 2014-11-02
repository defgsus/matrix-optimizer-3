/** @file object1tree.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.11.2014</p>
*/


#include "object1tree.h"
#include "object/object1.h"
#include "object/objectfactory.h"

namespace MO {

namespace Private
{

    QVariant getModelData(Object1 * obj, int role)
    {
        if (role == Qt::DisplayRole)
        {
            QString name = obj->infoName();
            if (obj->isModulated())
                name += "*";
            if (obj->canBeDeleted())
                return name;
            else
                return "{" + name + "}";
        }

        else if (role == Qt::EditRole)
            return obj->name();

        // icon
        if (role == Qt::DecorationRole)
        {
            //return ObjectFactory::iconForObject(obj);
        }

        return QVariant();
    }

    bool setModelData(Object1 * obj, const QVariant& value, int role)
    {
        if (role == Qt::EditRole)
            obj->setName(value.toString());

        return false;
    }

    void setObjectNode(Object1* o , Object1TreeNode * n)
    {
        o->p_set_node_(n);
    }

    void deleteObject(Object1* o)
    {
        delete o;
    }

} // namespace Private




class Object1Tree::Private
{
public:
    Object1 * root;
    Object1TreeNode * rootNode;
    //std::map<Object1*, Object1TreeNode*> nodes;
};



Object1Tree::Object1Tree(Object1 *root)
    : p_    (new Private())
{
    p_->root = root;
    p_->rootNode = new Object1TreeNode(p_->root, true);
    //p_->nodes.insert(std::make_pair(p_->root, p_->rootNode));
}

Object1Tree::~Object1Tree()
{
    delete p_;
}



void Object1Tree::addObject(Object1 * parent, Object1 * newChild, int insert_index)
{
    auto node = parent->node();
    MO_ASSERT(node, "No node in parent object, not part of ObjectTree");

    auto newnode = new Object1TreeNode(newChild);
    node->insert( insert_index, newnode );

    //p_->nodes.insert(std::make_pair(newChild, p_->rootNode));
}

void Object1Tree::deleteObject(Object1 * object)
{
    auto node = object->node();
    MO_ASSERT(node, "No node in parent object, not part of ObjectTree");

    if (node->parent())
        node->parent()->remove(object);
    else
        delete node;
}

void Object1Tree::swapChildren(Object1 * /*parent*/, int /*from*/, int /*to*/)
{
    //
}


















} // namespace MO

