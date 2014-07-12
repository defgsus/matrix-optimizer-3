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
#include "object/param/parameterfloat.h"
#include "object/scene.h"


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
    colorTransformation_ = QColor(0,60,120);
    colorTrack_ = QColor(90,90,90);
    colorSequence_ = QColor(0,120,0);
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
            if (obj->isTransformation())
                //|| (obj->parentObject() && obj->parentObject()->isTransformation()))
                return colorTransformation_;
            if (obj->isTrack())
                return colorTrack_;
            if (obj->isSequence())
                return colorSequence_;
            return colorDefault_;
        }

        // font
        if (role == Qt::FontRole)
        {
            if (!(obj->type() & Object::TG_MODULATION))
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
        data->storeObjectTree(obj);
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

    if (!data->formats().contains(ObjectTreeMimeData::objectMimeType))
    {
        MO_WARNING("mimedata is invalid");
        return false;
    }

    // NOTE: dynamic_cast or qobject_cast won't work between
    // application boundaries, e.g. after quit or pasting into
    // a different instance. But static_cast works alright.
    // It's important though to not rely on class members of ObjectTreeMimeData
    // but to manage everything over QMimeData::data()
    if (auto objdata = static_cast<const ObjectTreeMimeData*>(data))
    {
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
                        if (Scene * scene = obj->sceneObject())
                        {
                            ScopedTreeChange lock(scene, obj);
                            obj->addObject(fromObj, row);
                        }
                        else
                            obj->addObject(fromObj, row);

                        endMoveColumns();
                        lastDropIndex_ = createIndex(row, 0, copy);
                        return true;
                    }
                }

                // insert it
                beginInsertRows(parent, row, row);
                if (Scene * scene = obj->sceneObject())
                {
                    ScopedTreeChange lock(scene, obj);
                    obj->addObject(copy, row);
                }
                else
                    obj->addObject(copy, row);
                endInsertRows();

                lastDropIndex_ = createIndex(row, 0, copy);

                return true;
            }
            MO_WARNING("Could not deserialize object tree from mime-data");
            return false;
        }
        MO_WARNING("Object for drop index not found");
        return false;
    }
    MO_WARNING("mimedata is invalid");
    return false;
}




// ------------------- custom editing ----------------------

bool ObjectTreeModel::deleteObject(const QModelIndex & index)
{
    MO_DEBUG_TREE("ObjectTreeModel::deleteObject(" << index << ")");

    auto object = objectForIndex(index);
    if (!object)
        return false;
    auto parentIndex = parent(index);
    if (auto parentObject = objectForIndex(parentIndex))
    {
        if (Scene * scene = parentObject->sceneObject())
        {
            beginRemoveRows(parentIndex, index.row(), index.row());
            {
                ScopedTreeChange lock(scene, parentObject);
                parentObject->deleteObject(object);
            }
            endRemoveRows();
        }
        else
        {
            beginRemoveRows(parentIndex, index.row(), index.row());
            parentObject->deleteObject(object);
            endRemoveRows();
        }
        return true;
    }
    return false;
}

QModelIndex ObjectTreeModel::addObject(const QModelIndex &parentIndex, int row, Object * obj)
{
    MO_DEBUG_TREE("ObjectTreeModel::addObject(" << parentIndex
            << ", " << row << ", " << obj << ")");

    if (Object * parentObject = objectForIndex(parentIndex))
    {
        // adjust index
        if (row<0)
            row = parentObject->numChildren();

        beginInsertRows(parentIndex, row, row);

        if (Scene * scene = parentObject->sceneObject())
        {
            ScopedTreeChange lock(scene, parentObject);
            parentObject->addObject(obj, row);
        }
        else
            parentObject->addObject(obj, row);

        endInsertRows();

        return createIndex(row, 0, (void*)obj);
    }
    return QModelIndex();
}

bool ObjectTreeModel::addObject(Object *parent, Object * obj, int index)
{
    MO_DEBUG_TREE("ObjectTreeModel::addObject('" << parent->idName() << "', "
                  << index << ", " << obj << ")");

    // adjust index
    if (index<0)
        index = parent->numChildren();

    QModelIndex parentIndex = indexForObject(parent);

    beginInsertRows(parentIndex, index, index);

    if (Scene * scene = parent->sceneObject())
    {
        ScopedTreeChange lock(scene, parent);
        parent->addObject(obj, index);
    }
    else
        parent->addObject(obj, index);

    endInsertRows();

    return true;
}

QModelIndex ObjectTreeModel::swapChildren(Object *parent, int from, int to)
{
    MO_DEBUG_TREE("ObjectTreeModel::swapChildren(" << parent->idName()
                  << ", " << from << ", " << to << ")");

    if (from < 0 || from >= parent->numChildren()
        || to < 0 || to >= parent->numChildren()
        || from == to)
        return QModelIndex();

    QModelIndex parentIdx = indexForObject(parent);

    if (to < from)
        beginMoveRows(parentIdx, from, from, parentIdx, to);
    else
        beginMoveRows(parentIdx, to, to, parentIdx, from);

    if (Scene * scene = parent->sceneObject())
    {
        ScopedTreeChange lock(scene, parent);
        parent->swapChildren(from, to);
    }
    else
        parent->swapChildren(from, to);

    endMoveRows();

    return createIndex(to, 0, parent->childObjects()[to]);
}

QModelIndex ObjectTreeModel::moveUp(Object * object)
{
    QModelIndex idx = indexForObject(object);
    if (idx.row() < 1)
        return idx;
    return
        swapChildren(object->parentObject(), idx.row(), idx.row() - 1);
}

QModelIndex ObjectTreeModel::moveDown(Object * object)
{
    QModelIndex idx = indexForObject(object);
    if (idx.row() >= object->parentObject()->numChildren() - 1)
        return idx;
    return
        swapChildren(object->parentObject(), idx.row(), idx.row() + 1);
}

QModelIndex ObjectTreeModel::promote(Object *object)
{
    MO_DEBUG_TREE("ObjectTreeModel::promote(" << object->idName() << ")");

    QModelIndex idx = indexForObject(object);
    if (!idx.isValid())
        return idx;

    // can't go up
    if (!object->parentObject() || object->parentObject() == rootObject())
        return idx;

    // move from parent's child to grandparent's child
    Object * parent = object->parentObject();
    Object * grandParent = parent->parentObject();
    if (!grandParent)
        return idx;

    QModelIndex parentIdx = indexForObject(parent),
                grandParentIdx = indexForObject(grandParent);
    int fromRow = parent->childObjects().indexOf(object),
        toRow = grandParent->childObjects().indexOf(parent) + 1;

    // execute
    beginMoveRows(parentIdx, fromRow, fromRow, grandParentIdx, toRow);
    if (Scene * scene = grandParent->sceneObject())
    {
        ScopedTreeChange lock(scene, grandParent);
        grandParent->addObject(object, toRow);
    }
    else
        grandParent->addObject(object, toRow);
    endMoveRows();

    return indexForObject(object);
}

QModelIndex ObjectTreeModel::demote(Object *object)
{
    MO_DEBUG_TREE("ObjectTreeModel::demote(" << object->idName() << ")");

    QModelIndex idx = indexForObject(object);
    if (!idx.isValid())
        return idx;

    Object * parent = object->parentObject();
    if (!parent)
        return idx;

    Object * toObject;
    int toRow;
    int fromRow = parent->childObjects().indexOf(object);

    // add to below's childlist?
    if (fromRow == 0)
    {
        if (parent->numChildren() < 2)
            return idx;
        toObject = parent->childObjects()[1];
        toRow = 0;
    }
    // add to above's childlist?
    else
    {
        toObject = parent->childObjects()[fromRow-1];
        toRow = toObject->numChildren();
    }

    QModelIndex fromIdx = indexForObject(parent),
                toIdx = indexForObject(toObject);

    // execute
    beginMoveRows(fromIdx, fromRow, fromRow, toIdx, toRow);
    if (Scene * scene = parent->sceneObject())
    {
        ScopedTreeChange lock(scene, parent);
        toObject->addObject(object, toRow);
    }
    else
        toObject->addObject(object, toRow);
    endMoveRows();

    return indexForObject(object);
}


TrackFloat * ObjectTreeModel::createFloatTrack(ParameterFloat * param)
{
    MO_DEBUG_TREE("ObjectTreeModel::createFloatTrack('" << param->idName() << ")");

    Object * obj = param->object();

    // construct name
    QString name = obj->name() + "." + param->name();
    // if the parent is not a "real object" then use the grandparent as well
    if (!( (obj->type() & Object::TG_REAL_OBJECT)
           || (obj->type() & Object::TG_SEQUENCE))
        && obj->parentObject())
            name.prepend(obj->parentObject()->name() + ".");

    // find a place for the modulation track
    while (obj && !obj->canHaveChildren(Object::T_TRACK_FLOAT))
    {
        obj = obj->parentObject();
    }
    MO_ASSERT(obj, "Could not find an object to create a float track in.");

    // create track
    auto track = ObjectFactory::createTrackFloat(name);
    // add to parent
    addObject(indexForObject(obj), -1, track);

    // modulate parameter
    if (param->object() && param->object()->sceneObject())
    {
        ScopedObjectChange lock(param->object()->sceneObject(), param->object());
        param->addModulator(track->idName());
        param->collectModulators();
    }
    else
    {
        param->addModulator(track->idName());
        param->collectModulators();
    }

    return track;
}

SequenceFloat * ObjectTreeModel::createFloatSequence(TrackFloat *track, Double time)
{
    MO_DEBUG_TREE("ObjectTreeModel::createFloatSequence('" << track->idName() << "', " << time << ")");

    // get a good name
    QString name = track->name();
    if (name.contains("."))
    {
        int i = name.lastIndexOf(".");
        if (i < name.length()-1)
            name = name.mid(i+1);
    }

    QModelIndex trackIdx = indexForObject(track);

    // creat sequence
    auto seq = ObjectFactory::createSequenceFloat(name);

    // set properties
    seq->setStart(time);

    // place the sequence on the track
    addObject(trackIdx, -1, seq);

    return seq;
}

} // namespace MO
