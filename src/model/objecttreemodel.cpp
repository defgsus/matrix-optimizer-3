/** @file objecttreemodel.cpp

    @brief Tree Model for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include <QDebug>

#include "objecttreemodel.h"
#include "objecttreemimedata.h"
#include "io/error.h"
#include "io/log.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/trackfloat.h"
#include "object/sequencefloat.h"

namespace MO {

ObjectTreeModel::ObjectTreeModel(Object * rootObject, QObject *parent) :
    QAbstractItemModel(parent),
    rootObject_       (rootObject)
{
    headerNames_
            << "name"
            //<< "class"
            //<< "id"
            ;

    boldFont_.setBold(true);
    colorDefault_ = Qt::black;
    colorInvalid_ = Qt::red;
    colorTransformation_ = QColor(0,120,0);
}



void ObjectTreeModel::setRootObject(Object *rootObject)
{
    beginResetModel();

    rootObject_ = rootObject;

    endResetModel();
}

Object * ObjectTreeModel::objectForIndex(const QModelIndex &index) const
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
/*
void ObjectTreeModel::indexForObject(const Object *obj, QModelIndex& parent, int& row) const
{
    if (obj == rootObject_)
    {
        parent = QModelIndex();
        row = -1;
    }

    parent = createIndex(0, 0, obj->parentObject());
    row = obj->parentObject()->childObjects().indexOf((Object*)obj);
}
*/
QModelIndex ObjectTreeModel::indexForObject(const Object *obj) const
{
    if (obj == rootObject_)
    {
        return QModelIndex();//createIndex(-1,-1,(void*)rootObject_);
    }

    const Object * parent = obj->parentObject();
    const int idx = parent->childObjects().indexOf((Object*)obj);
    return createIndex(idx, 0, (void*)obj);
}


QModelIndex ObjectTreeModel::rootIndex() const
{
    if (!rootObject_)
        return QModelIndex();
    return createIndex(0,0, (void*)rootObject_);
}

QModelIndex ObjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (!rootObject_ || row < 0 || column < 0 || column >= headerNames_.size()
            //|| (parent.isValid() && column != 0)
            )
        return QModelIndex();

    Object * obj = objectForIndex(parent);
    MO_ASSERT(obj, "no object for index <"<<parent.row()<<","<<parent.column()<<">");

    if (row < obj->childObjects().size())
    {
        return createIndex(row, column, (void*)obj->childObjects()[row]);

    }

    return QModelIndex();
}

QModelIndex ObjectTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !rootObject_)
        return QModelIndex();

    if (Object * obj = objectForIndex(child))
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

    if (Object * obj = objectForIndex(parent))
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

    if (Object * obj = objectForIndex(index))
    {
        // return text
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            switch (index.column())
            {
                case 0: return role == Qt::DisplayRole? obj->infoName() : obj->name();
                        //QString::number(index.internalId(), 16);
                case 1: return obj->className();
                case 2: return obj->idName();
                default: MO_LOGIC_ERROR("no DisplayRole defined for column " << index.column());
            }
        }

        // object itself
        if (role == ObjectRole)
        {
            QVariant v;
            v.setValue(obj);
            return v;
        }

        // text alignment
        if (role == Qt::TextAlignmentRole)
            return (int)(Qt::AlignLeft | Qt::AlignVCenter);

        // color
        if (role == Qt::TextColorRole)
        {
            if (!obj->isValid())
                return colorInvalid_;
            if (obj->isTransformation() ||
                (obj->parentObject() && obj->parentObject()->isTransformation()))
                return colorTransformation_;
            return colorDefault_;
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
            return ObjectFactory::iconForObject(obj);
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


// ----------------------------- editing -----------------------------

bool ObjectTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!rootObject_ || !index.isValid() || index.column() < 0
            || index.column() >= headerNames_.size())
        return false;

    if (Object * obj = objectForIndex(index))
    {
        // set name
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
#ifdef MO_DISABLE_OBJECT_TREE_DRAG
    return 0;
#else
    return Qt::MoveAction | Qt::CopyAction;
#endif
}

Qt::DropActions ObjectTreeModel::supportedDropActions() const
{
#ifdef MO_DISABLE_OBJECT_TREE_DRAG
    return 0;
#else
    return Qt::MoveAction | Qt::CopyAction;
#endif
}

QStringList ObjectTreeModel::mimeTypes() const
{
    return ObjectTreeMimeData().formats();
}

QMimeData * ObjectTreeModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() != 1)
        return 0;

    if (Object * obj = objectForIndex(indexes.at(0)))
    {
        if (obj->isParameter())
            return 0;

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
    MO_DEBUG_TREE("ObjectTreeModel::dropMimeData(" << data << ", act(" << action
             << "), " << row << ", " << column << ", parent("
             << parent.row() << ", " << parent.column() << "))");

    if (action == Qt::IgnoreAction)
        return true;

    if (!(action == Qt::MoveAction ||
          action == Qt::CopyAction) || column > 0)
        return false;

    if (auto objdata = qobject_cast<const ObjectTreeMimeData*>(data))
    if (Object * obj = objectForIndex(parent))
    {
        if (row == -1)
            row = parent.isValid() ? parent.row()
                                   : rootObject_->numChildren();

        // deserialize object
        Object * copy = objdata->getObjectTree();

        // only act when it worked
        if (copy)
        {
            // rearrange if necessary
            //row = obj->getInsertIndex(copy, row);

            // delete previous tree
            if (action == Qt::MoveAction)
            {
                const QModelIndex& fromIndex = objdata->getModelIndex();
                const QModelIndex fromParentIndex =
                                    ObjectTreeModel::parent(fromIndex);
                if (Object * fromObj = objectForIndex(fromIndex))
                {
                    //if (Object * newp = itemForIndex(parent))
                    beginMoveColumns(fromParentIndex,
                                     fromIndex.row(), fromIndex.row(),
                                     parent, row);
                    MO_DEBUG_TREE("move " << fromObj->idName()
                                  << " " << fromIndex.row() << " -> "
                                  << obj->idName() << " " << row);
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

bool ObjectTreeModel::deleteObject(const QModelIndex & index)
{
    auto parentIndex = parent(index);
    if (auto parentObject = objectForIndex(parentIndex))
    {
        beginRemoveRows(parentIndex, index.row(), index.row());
        parentObject->deleteObject(objectForIndex(index));
        endRemoveRows();
        return true;
    }
    return false;
}

QModelIndex ObjectTreeModel::addObject(const QModelIndex &parentIndex, int row, Object * obj)
{
    MO_DEBUG_TREE("ObjectTreeModel::addObject_(parent("
             << parentIndex.row() << ", " << parentIndex.column() << "), "
             << row << ", " << obj << ")");

    if (Object * parentObject = objectForIndex(parentIndex))
    {
        // adjust index
        if (row<0)
            row = parentObject->numChildren();
        //row = parentObject->getInsertIndex(obj, row);

        beginInsertRows(parentIndex, row, row);
        parentObject->addObject(obj, row);
        endInsertRows();

        return createIndex(row, 0, (void*)obj);
    }
    return QModelIndex();
}


SequenceFloat * ObjectTreeModel::createFloatSequence(TrackFloat *track, Double time)
{
    MO_DEBUG_TREE("Scene::createFloatSequence('" << track->idName() << "', " << time << ")");

    QModelIndex trackIdx = indexForObject(track);

    // creat sequence
    auto * seq = ObjectFactory::createSequenceFloat();
    seq->setStart(time);

    // place the sequence somewhere
    addObject(trackIdx, -1, seq);

    // add it to track
    track->addSequence(seq);
    track->collectModulators();

    return seq;
}

} // namespace MO
