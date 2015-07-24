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

namespace MO {
namespace GUI {


PropertiesView::PropertiesView(QWidget *parent)
    : QWidget       (parent)
    , p_props_      (new Properties)
    , p_lv_         (0)
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

    if (!p_lv_)
    {
        // layout for property widgets
        p_lv_ = new QVBoxLayout(this);
        p_lv_->setMargin(1);
        p_lv_->setSizeConstraint(QLayout::SetMinAndMaxSize);
    }

    // create one for each property
    for (auto i = p_props_->begin(); i != p_props_->end(); ++i)
    {
        auto widget = new QVariantWidget(i.key(), p_props_, this);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        // keep track
        p_widgets_.insert(i.key(), widget);

        // install in layout
        p_lv_->addWidget(widget);

        // connect signals
        QString key = i.key();
        connect(widget, &QVariantWidget::valueChanged, [=]()
        {
            // copy to internal properties
            p_props_->set(key, widget->value());
            emit propertyChanged(key);
        });
    }

    setUpdatesEnabled(true);
}


} // namespace GUI
} // namespace MO
