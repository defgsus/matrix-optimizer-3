/** @file objectlistwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.11.2014</p>
*/

#include "objectlistwidget.h"
#include "object/object.h"
#include "object/objectfactory.h"

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
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(onDoubleClicked_(QListWidgetItem*)));
}


void ObjectListWidget::setParentObject(Object *parent)
{
    obj_ = parent;
    updateList_();
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
                        c->name(),
                        this,
                        IT_OBJECT);
        QVariant var;
        var.setValue(c);
        item->setData(Qt::UserRole, var);
        item->setForeground(QBrush(col));
        item->setBackground(QBrush(Qt::black));
        addItem(item);
    }
}

void ObjectListWidget::onDoubleClicked_(QListWidgetItem * item)
{
    if (!obj_)
        return;

    Object * nextSel = 0;

    if (item->type() == IT_ROOT)
        nextSel = obj_->rootObject();
    else if (item->type() == IT_DOTDOT)
        nextSel = obj_->parentObject();
    else
        nextSel = item->data(Qt::UserRole).value<Object*>();

    if (nextSel)
    {
        // XXX The new selected object will be triggered
        // by the containing ObjectView...
        //obj_ = nextSel;
        //updateList_();
        // ... upon this event
        emit objectSelected(nextSel);
    }
}

} // namespace GUI
} // namespace MO
