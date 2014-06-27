/** @file objecttreemodel.cpp

    @brief Tree Model for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include "objecttreemodel.h"
#include "io/error.h"
#include "object/object.h"

namespace MO {

ObjectTreeModel::ObjectTreeModel(Object * rootObject, QObject *parent) :
    QAbstractItemModel(parent),
    rootObject_       (rootObject)
{
    headerNames_
            << "name"
            << "class"
            << "id";
}

void ObjectTreeModel::setRootObject(Object *rootObject)
{
    beginResetModel();

    rootObject_ = rootObject;

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
    // invalid index means root object (??)
    return rootObject_;
}


QModelIndex ObjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (!rootObject_ || row < 0 || column < 0 || column >= headerNames_.size()
            //|| (parent.isValid() && column != 0)
            )
        return QModelIndex();

    Object * obj = itemForIndex(parent);
    MO_ASSERT(obj, "no object for index <"<<parent.row()<<","<<parent.column()<<">");

    if (row < obj->childObjects().size())
    {
        return createIndex(row, column, obj->childObjects()[row]);
    }

    return QModelIndex();
}

QModelIndex ObjectTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !rootObject_)
        return QModelIndex();

    if (Object * obj = itemForIndex(child))
    {
        // find parent object
        Object * parent = obj->parentObject();
        if (!parent || parent == rootObject_)
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
    if (!rootObject_)
        return 0;

    if (Object * obj = itemForIndex(parent))
        return obj->childObjects().size();

    return 0;
}


int ObjectTreeModel::columnCount(const QModelIndex &parent) const
{
    return (parent.isValid() && parent.column() != 0)? 0 : headerNames_.size();
}


QVariant ObjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!rootObject_ || !index.isValid() || index.column() < 0
            || index.column() >= headerNames_.size())
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
                case 2: return obj->idName();
                default: MO_LOGIC_ERROR("no DisplayRole defined for column " << index.column());
            }
        }

        // text alignment
        if (role == Qt::TextAlignmentRole)
            return (int)(Qt::AlignLeft | Qt::AlignVCenter);

        // icons
        if (role == Qt::DecorationRole && index.column() == 0)
        {
            return iconForObject(obj);
        }
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

    if (rootObject_ && index.isValid())
    {
        flag |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;

        if (index.column() == 0)
            flag |= Qt::ItemIsEditable;
    }
    return flag;
}

QVariant ObjectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal && section < headerNames_.size())
        {
            return headerNames_[section];
        }
    }

    return QVariant();
}


bool ObjectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!rootObject_ || !index.isValid() || index.column() < 0
            || index.column() >= headerNames_.size())
        return false;

    if (Object * obj = itemForIndex(index))
    {
        // return text
        if (role == Qt::EditRole && index.column() == 0)
        {
            obj->setName(value.toString());
            return true;
        }
    }

    return false;
}


const QIcon& ObjectTreeModel::iconForObject(const Object * o)
{
    static QIcon iconNone(":/icon/obj_none.png");
    static QIcon icon3d(":/icon/obj_3d.png");
    static QIcon iconParameter(":/icon/obj_parameter.png");
    static QIcon iconSoundSource(":/icon/obj_soundsource.png");
    static QIcon iconMicrophone(":/icon/obj_microphone.png");

    if (o->isMicrophone()) return iconMicrophone;
    if (o->isSoundSource()) return iconSoundSource;
    if (o->is3d()) return icon3d;
    if (o->isParameter()) return iconParameter;

    return iconNone;

}


} // namespace MO
