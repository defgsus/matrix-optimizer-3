/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/


#include <QColor>
#include <QBrush>
#include <QIcon>

#include "objecttreemodel.h"
#include "object/object.h"
#include "object/util/objectfactory.h"
#include "io/error.h"

namespace MO {

struct ObjectTreeModel::Private
{
    Private(ObjectTreeModel* p)
        : p         (p)
    {

    }

    ObjectTreeModel* p;

    Object * rootObject;
    QStringList headerNames;

};

ObjectTreeModel::ObjectTreeModel(Object * rootObject, QObject *parent) :
    QAbstractItemModel  (parent),
    p_                  (new Private(this))
{
    p_->headerNames
            << "name"
            //<< "class"
               ;
    setRootObject(rootObject);
}

Object* ObjectTreeModel::rootObject() const { return p_->rootObject; }

void ObjectTreeModel::setRootObject(Object *rootObject)
{
    beginResetModel();

    p_->rootObject = rootObject;

    endResetModel();
}

Object * ObjectTreeModel::itemForIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (auto o = static_cast<Object*>(index.internalPointer()))
        {
            return o;
        }
    }
    // invalid index means root object
    return p_->rootObject;
}


QModelIndex ObjectTreeModel::index(
        int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (!p_->rootObject || row < 0 || column < 0
            || column >= p_->headerNames.size()
            //|| (parent.isValid() && column != 0)
            )
        return QModelIndex();

    Object * obj = itemForIndex(parent);
    MO_ASSERT(obj, "no object for index <"
              << parent.row() << "," << parent.column() << ">");

    if (row < obj->childObjects().size())
    {
        return createIndex(row, column, obj->childObjects()[row]);
    }

    return QModelIndex();
}

QModelIndex ObjectTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !p_->rootObject)
        return QModelIndex();

    if (Object * obj = itemForIndex(child))
    {
        // find parent object
        Object * parent = obj->parentObject();
        if (!parent || parent == p_->rootObject)
            return QModelIndex();

        // get grandparent object
        if (Object * pparent = parent->parentObject())
        {
            // find index of child
            const int row = pparent->childObjects().indexOf(parent);
            if (row >= 0)
                return createIndex(row, 0, (void*)parent);
        }
    }

    return QModelIndex();
}

int ObjectTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!p_->rootObject)
        return 0;

    if (Object * obj = itemForIndex(parent))
        return obj->childObjects().size();

    return 0;
}


int ObjectTreeModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() && parent.column() != 0)
            ? 0 : p_->headerNames.size();
}


QVariant ObjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!p_->rootObject || !index.isValid() || index.column() < 0
            || index.column() >= p_->headerNames.size())
        return QVariant();

    if (Object * obj = itemForIndex(index))
    {
        // return text
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            switch (index.column())
            {
                case 0: return obj->name();
                case 1: return obj->className();
                default: MO_LOGIC_ERROR("no DisplayRole defined for column "
                                        << index.column());
            }
        }

        // text alignment
        if (role == Qt::TextAlignmentRole)
            return (index.column() == 0)?
                        (int)(Qt::AlignLeft | Qt::AlignVCenter)
                    :   (int)(Qt::AlignRight | Qt::AlignVCenter);
        /*
        if (role == Qt::BackgroundRole)
        {
            if (index.column() == 2)
                return QBrush(Qt::gray);
            if (index.column() == 1)
                return QBrush(Qt::green);
        }*/
    }

    return QVariant();
}

Qt::ItemFlags ObjectTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    if (p_->rootObject && index.isValid())
    {
        flag |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return flag;
}

QVariant ObjectTreeModel::headerData(
            int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal && section < p_->headerNames.size())
        {
            return p_->headerNames[section];
        }
    }

    return QVariant();
}


} // namespace MO
