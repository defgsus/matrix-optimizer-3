/** @file propertiesview.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#include <QLayout>
#include <QScrollArea>

#include "propertiesview.h"
#include "gui/widget/qvariantwidget.h"
#include "types/properties.h"

namespace MO {
namespace GUI {


PropertiesView::PropertiesView(QWidget *parent)
    : QWidget       (parent)
    , p_props_      (new Properties)
    , p_lv_         (0)
    , p_stretch_    (0)
    , p_scroll_     (0)
{

}

PropertiesView::~PropertiesView()
{
    delete p_props_;
}

void PropertiesView::setProperties(const Properties & p)
{
    *p_props_ = p;
    createWidgtes_();
}

void PropertiesView::clear()
{
    p_props_->clear();
    createWidgtes_();
}

void PropertiesView::createWidgtes_()
{
    // delete previous widgets
    for (auto w : p_widgets_)
    {
        w->setVisible(false);
        w->deleteLater();
    }
    p_widgets_.clear();
    if (p_stretch_)
        p_stretch_->deleteLater();

    // create scroll-area
    if (!p_scroll_)
    {
        auto lv = new QVBoxLayout(this);
        lv->setMargin(2);

        p_scroll_ = new QScrollArea(this);
        lv->addWidget(p_scroll_);

        p_container_ = new QWidget(p_scroll_);
        p_container_->setObjectName("_properties_container");
        p_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        // layout for property widgets
        p_lv_ = new QVBoxLayout(p_scroll_);
        p_lv_->setMargin(1);
        p_lv_->setSizeConstraint(QLayout::SetMinAndMaxSize);

        p_scroll_->setWidget(p_container_);
    }

    // create one for each property
    for (auto i = p_props_->begin(); i != p_props_->end(); ++i)
    {
        auto widget = new QVariantWidget(i.key(), i.value(), this);
        // keep track
        p_widgets_.insert(i.key(), widget);

        // install in layout
        p_lv_->addWidget(widget);

        // connect signals
        connect(widget, &QVariantWidget::valueChanged, [=]()
        {
            // copy to internal properties
            p_props_->set(i.key(), widget->value());
            emit propertyChanged(i.key());
        });
    }

    // a "stretch" that can be deleted later
    p_stretch_ = new QWidget(this);
    p_lv_->addWidget(p_stretch_);
    p_lv_->setStretch(p_lv_->indexOf(p_stretch_), 2);
}


} // namespace GUI
} // namespace MO
