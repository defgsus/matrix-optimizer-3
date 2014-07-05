/** @file objecttreesortproxy.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/

#include "objecttreesortproxy.h"
#include "object/object.h"


namespace MO {



ObjectTreeSortProxy::ObjectTreeSortProxy(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

/*
bool ObjectTreeSortProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    Object * l = static_cast<Object*>(left.internalPointer()),
           * r = static_cast<Object*>(right.internalPointer());

    return
        // transformations on top
        (l->isTransformation() && !r->isTransformation())
        ;
}
*/

} // namespace MO
