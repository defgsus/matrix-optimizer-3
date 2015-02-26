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

    if (!p_item_)
    {
        p_label_->clear();
        p_props_->clear();
    }
    else
    {
        p_label_->setText("XXX item");
        p_props_->setProperties(p_item_->properties());
    }
}

void FrontItemEditor::onPropertyChanged_(const QString &id)
{
    if (!p_item_)
        return;

    // send new property value to graphics item
    p_item_->setProperty(id, p_props_->properties().get(id));
}


} // namespace GUI
} // namespace MO
