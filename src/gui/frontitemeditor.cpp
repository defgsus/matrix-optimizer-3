/** @file frontitemeditor.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#include <QLayout>
#include <QLabel>

#include "frontitemeditor.h"
#include "item/abstractfrontitem.h"
#include "propertiesview.h"
#include "types/properties.h"

namespace MO {
namespace GUI {


FrontItemEditor::FrontItemEditor(QWidget *parent)
    : QWidget       (parent)
    , p_item_       (0)
    , p_props_      (0)
    , p_label_      (0)
{
    setObjectName("_FrontItemEditor");
}

void FrontItemEditor::setItem(AbstractFrontItem *item)
{
    p_item_ = item;
    p_items_.clear();
    updateWidgets_();
}

void FrontItemEditor::setItems(const QList<AbstractFrontItem *> &items)
{
    p_item_ = 0;
    p_items_ = items;
    updateWidgets_();
}

void FrontItemEditor::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        p_label_ = new QLabel(this);
        p_label_->setProperty("GroupHeader", true);
        lv->addWidget(p_label_);


        p_props_ = new PropertiesView(this);
        lv->addWidget(p_props_);

        connect(p_props_, SIGNAL(propertyChanged(QString)),
                this, SLOT(onPropertyChanged_(QString)));
}

void FrontItemEditor::updateWidgets_()
{
    if (!p_label_)
        createWidgets_();

    // unassign
    if (!isAssigned())
    {
        p_label_->clear();
        p_props_->clear();
    }
    else
    {
        // single item
        if (p_item_)
        {
            p_label_->setText(p_item_->name());
            p_props_->setProperties(p_item_->properties());
        }
        // multiple items
        else
        {
            p_label_->setText(tr("[%1 items]").arg(p_items_.size()));
            // extract properties list
            QList<Properties> props;
            for (auto i : p_items_)
                props << i->properties();
            p_props_->setProperties(props);
        }
    }
}

void FrontItemEditor::onPropertyChanged_(const QString &id)
{
    // send new property value to graphics item(s)
    if (p_item_)
        p_item_->setProperty(id, p_props_->properties().get(id));
    else
        // for multi-items, only change the property if existing
        for (auto i : p_items_)
            i->changeProperty(id, p_props_->properties().get(id));
}


} // namespace GUI
} // namespace MO
