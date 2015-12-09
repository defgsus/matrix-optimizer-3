/** @file propertiesscrollview.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#include <QLayout>

#include "propertiesscrollview.h"
#include "gui/widget/qvariantwidget.h"
#include "types/properties.h"

namespace MO {
namespace GUI {


PropertiesScrollView::PropertiesScrollView(QWidget *parent)
    : QScrollArea   (parent)
    , p_props_      (new Properties)
    , p_lv_         (0)
    , p_stretch_    (0)
    , p_container_  (0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

PropertiesScrollView::~PropertiesScrollView()
{
    delete p_props_;
}

void PropertiesScrollView::setProperties(const Properties & p)
{
    *p_props_ = p;

    createWidgtes_();
}

void PropertiesScrollView::setProperties(const QList<Properties>& list)
{
    p_props_->clear();
    for (auto & p : list)
        p_props_->unify(p);

    createWidgtes_();
}

void PropertiesScrollView::clear()
{
    p_props_->clear();
    createWidgtes_();
}

void PropertiesScrollView::createWidgtes_()
{
    setUpdatesEnabled(false);

    // delete previous widgets
    for (auto w : p_widgets_)
    {
        w->deleteLater();
    }
    p_widgets_.clear();
    if (p_stretch_)
    {
        p_stretch_->deleteLater();
        p_stretch_ = 0;
    }

    // widget container
    if (!p_container_)
    {
        p_container_ = new QWidget(this);
        p_container_->setObjectName("_properties_container");
        p_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        p_container_->setMinimumWidth(300);
        p_container_->setMaximumWidth(1024);
        setWidget(p_container_);

        // layout for property widgets
        p_lv_ = new QVBoxLayout(p_container_);
        p_lv_->setMargin(1);
        p_lv_->setSizeConstraint(QLayout::SetMinAndMaxSize);

    }

    auto props = p_props_->getSortedList();

    // create one for each property
    for (auto p : props)
    {
        auto widget = new QVariantWidget(p->id(), p_props_, p_container_);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        // keep track
        p_widgets_.insert(p->id(), widget);

        // install in layout
        p_lv_->addWidget(widget);

        // connect signals
        QString key = p->id();
        connect(widget, &QVariantWidget::valueChanged, [=]()
        {
            // copy to internal properties
            p_props_->set(key, widget->value());
            // get change to visibilty
            if (p_props_->callUpdateVisibility())
                updateWidgetVis_();
            emit propertyChanged(key);
        });
    }

    // a "stretch" that can be deleted later
    p_stretch_ = new QWidget(p_container_);
    p_lv_->addWidget(p_stretch_);
    p_lv_->setStretch(p_lv_->indexOf(p_stretch_), 2);


    setUpdatesEnabled(true);
}

void PropertiesScrollView::updateWidgetVis_()
{
//    MO_PRINT(p_props_->toString());
    for (auto i = p_props_->begin(); i != p_props_->end(); ++i)
    {
        auto j = p_widgets_.find(i.key());
        if (j != p_widgets_.end())
            j.value()->setVisible( i.value().isVisible() );
    }
}


} // namespace GUI
} // namespace MO
