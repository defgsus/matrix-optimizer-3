/** @file objecttreesortproxy.cpp

    @brief custom filter and sorted for ObjectTreeModel

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/
#include "io/memory.h"

#include <QDebug>

#include "objecttreesortproxy.h"
#include "objecttreemodel.h"
#include "object/object.h"


namespace MO {



ObjectTreeSortProxy::ObjectTreeSortProxy(QObject *parent) :
    QSortFilterProxyModel(parent),
    objectTypes_    (Object::TG_ALL)
{
}

bool ObjectTreeSortProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    Object * l = static_cast<Object*>(left.internalPointer()),
           * r = static_cast<Object*>(right.internalPointer());

    // keep original order
    const bool idxless = left.row() < right.row();

    // but sort for types (transformations on top)
    const bool less =
            (l->isTransformation() && r->isTransformation() && idxless)
            || (l->isTransformation() && !r->isTransformation())
            || (!l->isTransformation() && !r->isTransformation() && idxless);

//    qDebug() << left << right;
//    qDebug() << l << (less? "<" : ">") << r;
    return less;
}

void ObjectTreeSortProxy::setObjectTypes(int flags)
{
    objectTypes_ = flags;
    invalidateFilter();
}

bool ObjectTreeSortProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    Object * o = sourceModel()->data(idx, ObjectRole).value<Object*>();

    return o->type() & objectTypes_;
}



} // namespace MO
