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


ObjectTreeNode * getObjectTree(Object * root_object)
{
    auto node = new ObjectTreeNode(root_object);

    for (auto c : root_object->childObjects())
        node->append( getObjectTree(c) );

    return node;
}


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

} // namespace MO
