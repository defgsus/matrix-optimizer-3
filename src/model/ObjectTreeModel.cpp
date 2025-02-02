/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/


#include <QColor>
#include <QBrush>
#include <QIcon>
#include <QFileInfo>
#include <QUrl>
#include <QMessageBox>

#include "ObjectTreeModel.h"
#include "object/Object.h"
#include "gui/util/AppIcons.h"
#include "gui/util/ObjectMenu.h"
#include "object/util/ObjectFactory.h"
//#include "object/interface/masteroutinterface.h"
#include "object/util/ObjectEditor.h"
#include "model/ObjectMimeData.h"
#include "model/ObjectTreeMimeData.h"
#include "io/error.h"

#if 0
#   include "io/log.h"
#   include "io/streamoperators_qt.h"
#   define MO__D(arg__) MO_PRINT("ObjectTreeModel("<<this<<")::" << arg__)
#else
#   define MO__D(unused__) { }
#endif

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
    MO__D("ObjectTreeModel(" << rootObject << ", " << parent << ")");

    p_->headerNames
//            << "" // active
//            << "" // draw
            << tr("name")
            //<< "class"
               ;
    setRootObject(rootObject);
}

ObjectTreeModel::~ObjectTreeModel()
{
    MO__D("~ObjectTreeModel()");

    delete p_;
}

Object* ObjectTreeModel::rootObject() const { return p_->rootObject; }

void ObjectTreeModel::setRootObject(Object *rootObject)
{
    MO__D("setRootObject(" << rootObject << ")");

    beginResetModel();

    p_->rootObject = rootObject;

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
    // invalid index means root object
    return p_->rootObject;
}

QModelIndex ObjectTreeModel::indexForObject(Object* obj)
{
    if (!obj || !obj->parentObject())
        return QModelIndex();
    auto p = obj->parentObject();
    int row = p->childObjects().indexOf(obj);
    return createIndex(row, 0, obj);
}

QModelIndex ObjectTreeModel::index(
        int row, int column, const QModelIndex &parent) const
{
    // sanity check
    if (!p_->rootObject || row < 0 || column < 0
        || column >= p_->headerNames.size()
        )
        return QModelIndex();

    Object * obj = objectForIndex(parent);
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

    if (Object * obj = objectForIndex(child))
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

    if (Object * obj = objectForIndex(parent))
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

    if (Object * obj = objectForIndex(index))
    {
        // return text
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            switch (index.column())
            {
                /*
                case 0: return obj->activeAtAll();
                case 1:
                    if (auto mo = dynamic_cast<MasterOutInterface*>(obj))
                        return mo->isMasterOutputEnabled();
                    else
                        return false;
                */
                case 0: return obj->name();
                //case 1: return obj->className();
                default: MO_LOGIC_ERROR("no DisplayRole defined for column "
                                        << index.column());
            }
        }

        if (role == Qt::ForegroundRole)
        {
            if (obj->hasError())
                return QColor(255,0,0);
            else
                return ObjectFactory::colorForObject(obj);
        }

        if (role == Qt::DecorationRole && index.column() == 0)
        {
            return AppIcons::iconForObject(
                        obj, ObjectFactory::colorForObject(obj));
        }

    }

    return QVariant();
}

bool ObjectTreeModel::setData(
        const QModelIndex &index, const QVariant& val, int role)
{
    MO__D("setData(" << index << ", " << val.typeName() << ", " << role << ")");

    if (!p_->rootObject || !index.isValid() || index.column() < 0
            || index.column() >= p_->headerNames.size())
        return false;

    Object * obj = objectForIndex(index);
    if (!obj)
        return false;

    auto editor = obj->editor();

    if (role == Qt::EditRole)
    {
        if (index.column() == 0)
        {
            if (editor)
                editor->setObjectName(obj, val.toString());
            else
                obj->setName(val.toString());
            return true;
        }
    }

    emit dataChanged(index, index, QVector<int>() << role);

    return false;
}


Qt::ItemFlags ObjectTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);

    if (p_->rootObject && index.isValid())
    {
        flag |= Qt::ItemIsSelectable | Qt::ItemIsEnabled
                | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
                | Qt::ItemIsDropEnabled;
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


Qt::DropActions ObjectTreeModel::supportedDropActions() const
{
    return Qt::CopyAction |
            Qt::MoveAction;
}

QStringList ObjectTreeModel::mimeTypes() const
{
    return QStringList()
            << GUI::ObjectMenu::NewObjectMimeType
            << ObjectMimeData::mimeTypeString
            << ObjectTreeMimeData::objectMimeType
            << "text/uri-list";
}

QMimeData* ObjectTreeModel::mimeData(const QModelIndexList &indexes) const
{
    MO__D("mimeData(" << indexes.size() << ")");

    QSet<Object*> objs;
    for (auto& i : indexes)
        if (auto o = objectForIndex(i))
            objs << o;

    auto toplevel = ObjectTreeMimeData::filterTopLevel(objs);
    if (toplevel.isEmpty())
        return nullptr;

    auto data = new ObjectTreeMimeData();
    data->storeObjectTrees(toplevel);

    return data;
}

bool ObjectTreeModel::canDropMimeData(
        const QMimeData *data, Qt::DropAction action,
        int row, int /*column*/, const QModelIndex &parent) const
{
    //MO__D("canDropMimeData(" << data->formats() << ", act=" << action
    //      << ", row=" << row << ", parent=" << parent << ")");
    Q_UNUSED(action);

    if (!rootObject())
        return false;

    // check mime type
    bool sup = false;
    for (const auto& t : mimeTypes())
        sup |= data->formats().contains(t);
    if (!sup)
        return false;

    // get parent to drop into
    auto parentObj = objectForIndex(parent);
    if (!parentObj)
        parentObj = rootObject();
    if (!parentObj || !parentObj->editor())
        return false;

    // row out of range
    if (row >= 0 && parentObj->numChildren() >= 0
        && row > parentObj->numChildren())
    {
        MO_WARNING("ObjectTreeModel::dropMimeData() row out of range: "
                   << row << "/" << parentObj->numChildren());
        return false;
    }

    // see if object fits
    if (data->formats().contains(GUI::ObjectMenu::NewObjectMimeType))
    {
        // get object type
        auto classn = QString::fromUtf8(
                    data->data(GUI::ObjectMenu::NewObjectMimeType));
        int typ = ObjectFactory::typeForClass(classn);
        // check if dropable
        if (typ < 0 || !parentObj->canHaveChildren(Object::Type(typ)))
            return false;
    }

    return true;
}

bool ObjectTreeModel::dropMimeData(
        const QMimeData *data, Qt::DropAction action,
        int row, int column, const QModelIndex &parent)
{
    MO__D("dropMimeData(" << data->formats() << ", act=" << action << ", row="
          << row << ", parent=" << parent << ")");

    if (!canDropMimeData(data, action, row, column, parent))
    {
        MO__D("Quit drop becaue canDropMimeData() returned false");
        return false;
    }

    auto parentObj = objectForIndex(parent);
    if (!parentObj)
        parentObj = rootObject();

    QList<Object*> newObjects;
    QStringList errors;

    // drop object trees
    if (data->formats().contains(ObjectTreeMimeData::objectMimeType))
    {
        auto objdata = static_cast<const ObjectTreeMimeData*>(data);

        // catch move events
        if (action == Qt::MoveAction)
        {
            auto ids = objdata->getObjectTreeIds();
            QList<Object*> objs;
            for (auto id : ids)
                if (auto o = rootObject()->findChildObject(id, true))
                    objs << o;
            if (objs.isEmpty())
            {
                MO__D("Quit drop, because found no objects belonging to "
                      "the IDs to be moved");
                return false;
            }
            auto editor = parentObj->editor();
            MO_ASSERT(editor, "");
            for (auto o : objs)
                editor->moveObject(o, parentObj, row++);
            return true;
        }

        newObjects = objdata->getObjectTrees();
        MO__D("Got " << newObjects.size() << " objects from objectMimeType");
    }

    // drop new-object
    else if (data->formats().contains(GUI::ObjectMenu::NewObjectMimeType))
    {
        // get object type
        auto classn = QString::fromUtf8(
                    data->data(GUI::ObjectMenu::NewObjectMimeType));
        int typ = ObjectFactory::typeForClass(classn);
        // check if dropable
        if (typ < 0 || !parentObj->canHaveChildren(Object::Type(typ)))
        {
            MO__D("Quit drop because NewObjectMimeType does not fit here");
            return false;
        }
        if (auto obj = ObjectFactory::createObject(classn))
            newObjects << obj;
    }
    // drop urls
    if (data->hasUrls())
    {
        const auto list = data->urls();
        for (const QUrl& url : list)
        {
            QString shortfn = QFileInfo(url.toString()).fileName();
            try
            {
                if (auto o = ObjectFactory::createObjectFromUrl(url))
                    newObjects << o;
                else
                    errors << QObject::tr("%1: format not supported")
                                  .arg(shortfn);
            }
            catch (const Exception& e)
            {
                errors << QObject::tr("%1: %2")
                              .arg(shortfn)
                              .arg(e.what());
            }
        }
    }

    bool added = false;
    if (!newObjects.isEmpty())
    {
        added = parentObj->editor()->addObjects(parentObj, newObjects, row);
        if (added)
            MO__D("dropped " << newObjects.size() << " objects")
        else
            MO__D("dropping of " << newObjects.size() << " objects failed");
    }

    // print errors
    if (!errors.isEmpty())
    {
        QString msg = QObject::tr(
                    "The following urls could not be wrapped into an object.");
        for (const auto & text : errors)
        {
            msg.append("\n" + text);
        }
        QMessageBox::information(0, QObject::tr("File drop"), msg);
    }

    //return !newObjects.empty();
    return added && errors.isEmpty();
}


#if 0
bool ObjectTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    MO__D("insertRows(row=" << row << ", cnt=" << count
          << ", parent=" << parent << ")");
    return false;
}

bool ObjectTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    MO__D("removeRows(row=" << row << ", cnt=" << count
          << ", parent=" << parent << ")");

    auto pobj = objectForIndex(parent);
    if (!pobj || !pobj->editor())
        return false;

    QList<Object*> objs;
    for (int i=0; i<count && row < pobj->numChildren(); ++i, ++row)
        objs << pobj->childObjects()[row];
    if (objs.isEmpty())
        return false;

    beginResetModel();
    pobj->editor()->deleteObjects(objs);
    endResetModel();

    return true;
}

bool ObjectTreeModel::moveRows(
        const QModelIndex &sourceParent, int sourceRow, int count,
        const QModelIndex &destinationParent, int destinationChild)
{
    MO_PRINT("moveRows(" << sourceParent << ", " << sourceRow << ", " << count
             << ", " << destinationParent << ", " << destinationChild << ")");
    return false;
}
#endif


} // namespace MO
