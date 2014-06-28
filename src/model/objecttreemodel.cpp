/** @file objecttreemodel.cpp

    @brief Tree Model for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include <QDebug>

#include "objecttreemodel.h"
#include "objecttreemimedata.h"
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

    boldFont_.setBold(true);
}

const QIcon& ObjectTreeModel::iconForObject(const Object * o)
{
    static QIcon iconNone(":/icon/obj_none.png");
    static QIcon icon3d(":/icon/obj_3d.png");
    static QIcon iconParameter(":/icon/obj_parameter.png");
    static QIcon iconSoundSource(":/icon/obj_soundsource.png");
    static QIcon iconMicrophone(":/icon/obj_microphone.png");
    static QIcon iconCamera(":/icon/obj_camera.png");

    if (o->isCamera()) return iconCamera;
    if (o->isMicrophone()) return iconMicrophone;
    if (o->isSoundSource()) return iconSoundSource;
    if (o->is3d()) return icon3d;
    if (o->isParameter()) return iconParameter;

    return iconNone;

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

        // color
        if (role == Qt::TextColorRole)
        {
            if (!obj->isValid())
                return QColor(Qt::red);
        }

        // font
        if (role == Qt::FontRole)
        {
            if (index.column() == 0)
                return boldFont_;
        }

        // icon
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
            flag |= Qt::ItemIsEditable
                    | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
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

// ---------------------------- DRAG/DROP ---------------------------------

Qt::DropActions ObjectTreeModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions ObjectTreeModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList ObjectTreeModel::mimeTypes() const
{
    return ObjectTreeMimeData().formats();
}

QMimeData * ObjectTreeModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() != 1)
        return 0;

    if (Object * obj = itemForIndex(indexes.at(0)))
    {
        auto data = new ObjectTreeMimeData();
        data->setModelIndex(indexes.at(0));
        data->setObjectTree(obj);
        return data;
    }

    return 0;
}

bool ObjectTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent)
{
    //qDebug() << "index " << action << row << column << parent.row() << parent.column();

    if (action == Qt::IgnoreAction)
        return true;

    if (!(action == Qt::MoveAction ||
          action == Qt::CopyAction) || column > 0)
        return false;

    if (auto objdata = qobject_cast<const ObjectTreeMimeData*>(data))
    if (Object * obj = itemForIndex(parent))
    {
        if (row == -1)
            row = parent.isValid() ? parent.row()
                                   : rootObject_->childObjects().count();

        // deserialize object
        Object * copy = objdata->getObjectTree();

        // only act when this worked
        if (copy)
        {
            // delete previous tree
            if (action == Qt::MoveAction)
            {
                const QModelIndex& fromIndex = objdata->getModelIndex();
                const QModelIndex fromParentIndex =
                                    ObjectTreeModel::parent(fromIndex);
                if (Object * fromObj = itemForIndex(fromIndex))
                {
                    //if (Object * newp = itemForIndex(parent))
                    beginMoveColumns(fromParentIndex,
                                     fromIndex.row(), fromIndex.row(),
                                     parent, row);
                    qDebug() << "move" << fromObj->idName()
                             << fromIndex.row() << "->"
                             << obj->idName() << row;
                    obj->addObject(fromObj, row);
                    endMoveColumns();
                    return true;
                }
            }

            // insert it
            beginInsertRows(parent, row, row);
            obj->addObject(copy, row);
            endInsertRows();
        }
        return true;
    }
    return false;
}


// ------------------- custom editing ----------------------

void ObjectTreeModel::deleteObject(const QModelIndex & index)
{
    auto parentIndex = parent(index);
    if (auto parentObject = itemForIndex(parentIndex))
    {
        beginRemoveRows(parentIndex, index.row(), index.row());
        parentObject->deleteObject(itemForIndex(index));
        endRemoveRows();
    }
}



} // namespace MO
