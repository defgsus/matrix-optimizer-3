/** @file objectlistwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.11.2014</p>
*/

#include <QDebug>
#include <QDropEvent>

#include "ObjectListWidget.h"
#include "ObjectListWidgetItem.h"
#include "object/Object.h"
#include "object/Scene.h"
#include "object/util/ObjectFactory.h"
#include "object/util/ObjectEditor.h"
#include "model/ObjectTreeMimeData.h"
#include "io/error.h"
#include "io/log_gui.h"

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
    MO_DEBUG_GUI("ObjectListWidget::setParentObject(" << (void*)parent << ")");

    Object * sel = 0;

    // rather display the parent if this object is empty
    if (parent)
    {
        // XXX For now, always show/select parent
        if (parent->childObjects().isEmpty()
            && parent->parentObject())
        {
            sel = parent; //< select the original object
            parent = parent->parentObject();
        }
    }

    obj_ = parent;

    // Query the ObjectEditor
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
        connect(editor, SIGNAL(objectNameChanged(MO::Object*)),
                this, SLOT(onObjectNamedChanged_(MO::Object*)));
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
//        item->setForeground(QBrush(Qt::white));
//        item->setBackground(QBrush(Qt::black));
        addItem(item);

        item = new QListWidgetItem("..", this, IT_DOTDOT);
//        item->setForeground(QBrush(Qt::white));
//        item->setBackground(QBrush(Qt::black));
        addItem(item);
    }

    for (Object * c : obj_->childObjects())
    {
        if (c->isVisible())
        {
            auto item = new ObjectListWidgetItem(c, this, IT_OBJECT);
            addItem(item);
        }

        // show the invisible objects in debug mode
#ifndef NDEBUG
        if (!c->isVisible())
        {
            auto item = new ObjectListWidgetItem(c, this, IT_OBJECT);
            item->setBackgroundColor(QColor(40,20,20));
            addItem(item);
        }
#endif
    }
}

Object * ObjectListWidget::objectForItem(const QListWidgetItem * item) const
{
    if (root_ && item->type() == IT_OBJECT)
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
            setCurrentItem(it);
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

void ObjectListWidget::onObjectDeleted_(const Object * o)
{
    Q_UNUSED(o);
    MO_DEBUG_GUI("ObjectListWidget::onObjectDeleted(" << (void*)o << ")");

    // XXX mhhh...
    setParentObject(0);
}

void ObjectListWidget::onObjectNamedChanged_(Object * )
{
    updateList_();
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
        if (!dropObject || !dropObject->parentObject())
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
