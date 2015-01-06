/** @file objecttree.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.10.2014</p>
*/

#include "objecttree.h"
#include "object/object.h"
#include "object/objectfactory.h"

namespace MO {


ObjectTreeNode * get_object_tree(Object * root_object, bool own)
{
    auto node = new ObjectTreeNode(root_object, own);

    for (auto c : root_object->childObjects())
        node->append( get_object_tree(c, own) );

    return node;
}



#if 0

namespace Private
{

    QVariant getModelData(Object * obj, int role)
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
            return ObjectFactory::iconForObject(obj);
        }

        return QVariant();
    }

    bool setModelData(Object * obj, const QVariant& value, int role)
    {
        if (role == Qt::EditRole)
            obj->setName(value.toString());

        return false;
    }

} // namespace Private




class ObjectTree::Private
{
public:
    Object * root;
    ObjectTreeNode * rootNode;
    std::map<Object*, ObjectTreeNode*> nodes;
};



ObjectTree::ObjectTree(Object *root)
    : p_    (new Private())
{
    p_->root = root;
    p_->rootNode = new ObjectTreeNode(p_->root, true);
    p_->nodes.insert(std::make_pair(p_->root, p_->rootNode));
}

ObjectTree::~ObjectTree()
{
    delete p_;
}



#endif















} // namespace MO
