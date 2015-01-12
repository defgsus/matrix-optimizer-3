/** @file objectlistwidgetitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.11.2014</p>
*/
#include <QDebug>
#include "objectlistwidgetitem.h"
#include "objectlistwidget.h"
#include "gui/util/appicons.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/scene.h"
#include "object/util/objecteditor.h"

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
        if (object_->childObjects().isEmpty())
            return object_->name();
        else
            return object_->name() + " ...";
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
