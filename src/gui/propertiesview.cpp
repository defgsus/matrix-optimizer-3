/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2015</p>
*/

#include <QLayout>

#include "propertiesview.h"
#include "gui/widget/qvariantwidget.h"
#include "types/properties.h"
#include "io/log.h"
namespace MO {
namespace GUI {


PropertiesView::PropertiesView(QWidget *parent)
    : QWidget       (parent)
    , p_props_      (new Properties)
    , p_lg_         (0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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

void PropertiesView::setProperties(const QList<Properties>& list)
{
    p_props_->clear();
    for (auto & p : list)
        p_props_->unify(p);

    createWidgtes_();
}

void PropertiesView::clear()
{
    p_props_->clear();
    createWidgtes_();
}

void PropertiesView::createWidgtes_()
{
    setUpdatesEnabled(false);

    // delete previous widgets
    for (auto w : p_widgets_)
    {
#if 0
        w->deleteLater();
#else
        // XXX For some reasons, widgets dont get removed properly
        w->setParent(0);
        delete w;
#endif
    }
    p_widgets_.clear();

    if (!p_lg_)
    {
        // layout for property widgets
        p_lg_ = new QGridLayout(this);
        p_lg_->setMargin(1);
        p_lg_->setSizeConstraint(QLayout::SetMinAndMaxSize);
    }

    // create one for each property
    for (auto i = p_props_->begin(); i != p_props_->end(); ++i)
    {
        auto widget = new QVariantWidget(i.key(), p_props_, this);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        // keep track
        p_widgets_.insert(i.key(), widget);

        // install in layout
        p_lg_->addWidget(widget, i.value().index(), 0, 1, 1);

        // connect signals
        QString key = i.key();
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

    // initial visibility
    p_props_->callUpdateVisibility();
    updateWidgetVis_();

    setUpdatesEnabled(true);
}

void PropertiesView::updateWidgetVis_()
{
    MO_PRINT(p_props_->toString());
    for (auto i = p_props_->begin(); i != p_props_->end(); ++i)
    {
        auto j = p_widgets_.find(i.key());
        if (j != p_widgets_.end())
            j.value()->setVisible( i.value().isVisible() );
    }
}


} // namespace GUI
} // namespace MO
