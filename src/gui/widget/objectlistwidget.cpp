/** @file objectlistwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.11.2014</p>
*/

#include <QDebug>
#include <QDropEvent>

#include "objectlistwidget.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/objectfactory.h"
#include "object/util/objecteditor.h"
#include "model/objecttreemimedata.h"
#include "io/error.h"

namespace MO {
namespace GUI {

namespace {
    enum ItemType
    {
        IT_ROOT,
        IT_DOTDOT,
        IT_OBJECT
    };
}

ObjectListWidget::ObjectListWidget(QWidget *parent)
    : QListWidget   (parent),
      obj_          (0)
{
    setDragDropMode(DragDrop);

    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(onDoubleClicked_(QListWidgetItem*)));
    connect(this, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(onItemSelected_(QListWidgetItem*)));
}


void ObjectListWidget::setParentObject(Object *parent)
{
    Object * sel = 0;

    // rather display the parent if this object is empty
    if (parent)
    {
        if (parent->childObjects().isEmpty()
            && parent->parentObject())
        {
            sel = parent; //< select the original object
            parent = parent->parentObject();
        }
    }

    obj_ = parent;

    root_ = obj_ ? obj_->rootObject() : 0;
    auto editor = root_ && root_->type() == Object::T_SCENE ?
                static_cast<Scene*>(root_)->editor()
              : 0;

    if (editor && editor != editor_)
    {
        connect(editor, SIGNAL(objectAdded(MO::Object*)),
                this, SLOT(onObjectAdded_(MO::Object*)));
        connect(editor, SIGNAL(objectChanged(MO::Object*)),
                this, SLOT(onObjectChanged_(MO::Object*)));
        connect(editor, SIGNAL(objectDeleted(const MO::Object*)),
                this, SLOT(onObjectDeleted_(const MO::Object*)));
    }
    editor_ = editor;

    updateList_();

    if (sel)
        setSelectedObject(sel);
}

void ObjectListWidget::updateList_()
{
    clear();
    if (!obj_)
        return;

    if (obj_->parentObject())
    {
        auto item = new QListWidgetItem("/", this, IT_ROOT);
        item->setForeground(QBrush(Qt::white));
        item->setBackground(QBrush(Qt::black));
        addItem(item);

        item = new QListWidgetItem("..", this, IT_DOTDOT);
        item->setForeground(QBrush(Qt::white));
        item->setBackground(QBrush(Qt::black));
        addItem(item);
    }

    for (auto c : obj_->childObjects())
    {
        QColor col = ObjectFactory::colorForObject(c);

        auto item = new QListWidgetItem(
                        ObjectFactory::iconForObject(c, col),
                        c->name() + (c->childObjects().isEmpty() ? "" : " ..."),
                        this,
                        IT_OBJECT);
        //QVariant var;
        //var.setValue(c);
        //item->setData(Qt::UserRole, var);
        item->setData(Qt::UserRole, c->idName());
        item->setForeground(QBrush(col));
        item->setBackground(QBrush(Qt::black));
        item->setFlags(Qt::ItemIsEnabled
                       | Qt::ItemIsSelectable
                       | Qt::ItemIsDragEnabled
                       | Qt::ItemIsDropEnabled);
        addItem(item);
    }
}

Object * ObjectListWidget::objectForItem(const QListWidgetItem * item) const
{
    if (root_)
    {
        return root_->findChildObject(
            item->data(Qt::UserRole).toString(),
            true);
    }
    return 0;
}

void ObjectListWidget::onDoubleClicked_(QListWidgetItem * item)
{
    if (!obj_)
        return;

    Object * nextSel = 0;

    if (item->type() == IT_ROOT)
        nextSel = root_;
    else if (item->type() == IT_DOTDOT)
        nextSel = obj_->parentObject();
    else
        nextSel = objectForItem(item);

    if (nextSel)
    {
        // XXX The new selected object will be triggered
        // by the containing ObjectView...
//      setParentObject(nextSel);
        // ... upon this event
        emit objectSelected(nextSel);
    }
}

void ObjectListWidget::onItemSelected_(QListWidgetItem * item)
{
    auto o = objectForItem(item);
    if (o)
        emit objectClicked(o);
}

void ObjectListWidget::setSelectedObject(Object *o)
{
    if (!o)
    {
        clearSelection();
        return;
    }

    for (int i = 0; i < count(); ++i)
    {
        auto it = item(i);
        if (it->data(Qt::UserRole).toString() == o->idName())
        {
            it->setSelected(true);
            return;
        }
    }
}

// --------------------------- signals from editor ---------------------------

void ObjectListWidget::onObjectChanged_(Object * )
{
    updateList_();
}

void ObjectListWidget::onObjectAdded_(Object * )
{
    updateList_();
}

void ObjectListWidget::onObjectDeleted_(const Object *)
{
    // XXX mhhh...
    setParentObject(0);
}

// ------------------------------- clipboard ---------------------------------

QStringList ObjectListWidget::mimeTypes() const
{
    return QStringList() << ObjectTreeMimeData::objectMimeType;
}

QMimeData * ObjectListWidget::mimeData(const QList<QListWidgetItem *> items) const
{
    // get an Object* list for item list
    QList<Object*> list;
    for (auto i : items)
        if (auto o = objectForItem(i))
            list << o;

    if (list.isEmpty())
        return 0;

    // serialize objects into special mime-data
    auto data = new ObjectTreeMimeData();
    data->storeObjectTrees(list);

    return data;
}

void ObjectListWidget::dropEvent(QDropEvent * e)
{
    e->ignore();

    if (!obj_ || !editor_)
        return;

    auto dropItem = itemAt(e->pos());
    if (!dropItem)
        return;

    Object * dropObject = objectForItem(dropItem);

    // dropping serialized objects?
    if (e->mimeData()->formats().contains(ObjectTreeMimeData::objectMimeType))
    {
        if (!dropObject && !dropObject->parentObject())
            return;

        auto data = static_cast<const ObjectTreeMimeData*>(e->mimeData());

        // move action
        if (1)
        {
            Object * newParent = dropObject->parentObject();
            // index of dropObject in it's parent
            int idx = newParent->childObjects().indexOf(dropObject);

            // get all ids of objects to move
            QList<QString> ids = data->getObjectTreeIds();
            for (auto & id : ids)
            {
                // find original object
                auto orgObj = root_->findChildObject(id, true);
                if (!orgObj)
                {
                    MO_WARNING("Did not find orignal object '" << id << "' in ObjectListWidget::dropEvent()");
                    continue;
                }

                // move each object
                editor_->moveObject(orgObj, newParent, idx++);
            }
        }

        // paste action
        else
        {
            // index of dropObject in it's parent
            int idx = dropObject->parentObject()->childObjects().indexOf(dropObject);

            auto newObjects = data->getObjectTrees();

            // insert before dropobject
            editor_->addObjects(dropObject->parentObject(),
                                newObjects, idx);
        }

        e->accept();
    }
}

} // namespace GUI
} // namespace MO
