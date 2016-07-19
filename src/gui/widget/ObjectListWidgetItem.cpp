/** @file objectlistwidgetitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.11.2014</p>
*/
#include <QDebug>
#include "ObjectListWidgetItem.h"
#include "ObjectListWidget.h"
#include "gui/util/AppIcons.h"
#include "object/Object.h"
#include "object/util/ObjectFactory.h"
#include "object/Scene.h"
#include "object/util/ObjectEditor.h"

namespace MO {
namespace GUI {

ObjectListWidgetItem::ObjectListWidgetItem(Object *o, ObjectListWidget *parent, int type)
    : QListWidgetItem(parent, type),
      object_   (o)
{
    const auto col = ObjectFactory::colorForObject(object_);

    setIcon(AppIcons::iconForObject(object_, col));
    setData(Qt::UserRole, object_->idName());
    setForeground(QBrush(col));
//    setBackground(QBrush(Qt::black));
    setFlags(Qt::ItemIsEnabled
           | Qt::ItemIsSelectable
           | Qt::ItemIsDragEnabled
           | Qt::ItemIsDropEnabled
           | Qt::ItemIsEditable);
}


QVariant ObjectListWidgetItem::data(int role) const
{
    if (role == Qt::DisplayRole)
    {
        QString t = object_->isVisible() ?
                    object_->name()
                  : QString("{%1}").arg(object_->name());
        if (object_->childObjects().isEmpty())
            return t;
        else
            return t + " ...";
    }
    if (role == Qt::EditRole)
        return object_->name();
    if (role == Qt::ToolTipRole)
        return object_->namePath();

    return QListWidgetItem::data(role);
}

void ObjectListWidgetItem::setData(int role, const QVariant &value)
{
    if (role == Qt::EditRole)
    {
        if (auto scene = object_->sceneObject())
            if (auto editor = scene->editor())
            {
                editor->setObjectName(object_, value.toString());
                return;
            }

        object_->setName(value.toString());
        return;
    }

    QListWidgetItem::setData(role, value);
}


} // namespace GUI
} // namespace MO
