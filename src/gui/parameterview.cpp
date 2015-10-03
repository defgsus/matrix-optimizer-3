/** @file parameterview.cpp

    @brief Display and editor for Object parameters

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QAbstractItemView>
#include <QScrollArea>
#include <QScrollBar>
#include <QDebug>
#include "parameterview.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/control/sequencefloat.h"
#include "object/util/objecteditor.h"
#include "object/param/parameters.h"
#include "object/param/parameter.h"
#include "object/param/modulator.h"
#include "io/error.h"
#include "io/log.h"
#include "widget/parameterwidget.h"
#include "widget/groupwidget.h"
#include "modulatordialog.h"
#include "util/scenesettings.h"

namespace MO {
namespace GUI {

ParameterView::ParameterView(QWidget *parent) :
    QWidget         (parent),
    scene_          (0),
    editor_         (0),
    object_         (0),
    p_stretch_      (0),
    doStoreScrollbarPos_ (false),
    curScrollbarValue_(0)
{
    setObjectName("_ParameterView");

    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSizeConstraint(QLayout::SetMaximumSize);//SetMinAndMaxSize);

        scrollArea_ = new QScrollArea(this);
        if (auto sb = scrollArea_->verticalScrollBar())
            connect(sb, SIGNAL(valueChanged(int)),
                this, SLOT(onScrollBarChange_(int)));
        scrollArea_->viewport()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        layout->addWidget(scrollArea_);

            container_ = new QWidget(scrollArea_);
            container_->setObjectName("_parameter_container");
            container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

            layout_ = new QVBoxLayout(container_);
            layout_->setMargin(0);
            layout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

            scrollArea_->setWidget(container_);
}



void ParameterView::setObject(Object *object)
{
    object_ = object;

    if (!object_)
    {
        clearWidgets_();
        return;
    }

    Scene * scene = object_->sceneObject();
    if (scene != scene_)
    {
        scene_ = scene;
        editor_ = scene_->editor();
        connect(editor_, SIGNAL(parameterChanged(MO::Parameter*)),
                this, SLOT(updateWidgetValue_(MO::Parameter*)));
        connect(editor_, SIGNAL(parametersChanged()),
                this, SLOT(updateParameters()));
        connect(editor_, SIGNAL(sequenceChanged(MO::Sequence*)),
                this, SLOT(onSequenceChanged(MO::Sequence*)));
    }

    parameters_ = object_->params()->parameters();

    createWidgets_();
}

void ParameterView::updateParameterVisibility(Parameter * p)
{
    // see if parameter is known
    auto i = paramMap_.find(p);
    if (i!=paramMap_.end())
    {
        // see if there's a group
        auto j = groups_.find(p->groupId());
        if (j != groups_.end())
            j.value()->setVisible(i.value(), p->isVisible());
        // otherwise change widget directly
        else
            i.value()->setVisible(p->isVisible());

        //squeezeView_();
    }
}

#if 0
// XXX That's probably not like Qt people imagined
void ParameterView::squeezeView_()
{
    const int h = scrollArea_->verticalScrollBar()->sliderPosition();

    for (auto g : groups_)
        g->layout()->activate();
    layout_->activate();

    scrollArea_->widget()->setGeometry(QRect(0,0,
                            scrollArea_->viewport()->width(),1));

    scrollArea_->ensureWidgetVisible(scrollArea_->widget()->focusWidget());

    // little hack to update the viewport to the slider position
    // (it won't do it without)
    scrollArea_->verticalScrollBar()->setSliderPosition(h-1);
    scrollArea_->verticalScrollBar()->setSliderPosition(h);
}
#endif

void ParameterView::resizeEvent(QResizeEvent *)
{
    QRect r = scrollArea_->widget()->geometry();
    r.setWidth(scrollArea_->viewport()->width());
    scrollArea_->widget()->setGeometry(r);
}


void ParameterView::clearWidgets_()
{
    doStoreScrollbarPos_ = false;

    bool doUpdate = updatesEnabled();
    setUpdatesEnabled(false);

#if 1
    for (auto w : widgets_)
        w->deleteLater();
    widgets_.clear();

    for (auto g : groups_)
    {
        if (!g)
        {
            MO_WARNING("This segfaulted once!!!!\n" __FILE__ ": " << __LINE__);
            continue;
        }
        g->deleteLater();
    }
    groups_.clear();

    if (p_stretch_)
    {
        p_stretch_->deleteLater();
        p_stretch_ = 0;
    }

    // XXX Below is more elegant
    // but then QScrollBar->setValue() does not work for restoring
    // the last scrollbar position (in createWidgets_())
    // For whatever reason...
#else
    container_->deleteLater();
    container_ = new QWidget(scrollArea_);
    container_->setObjectName("_parameter_container");
    container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout_ = new QVBoxLayout(container_);
    layout_->setMargin(0);
    layout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
    scrollArea_->setWidget(container_);

    widgets_.clear();
    groups_.clear();
#endif

    paramMap_.clear();

    if (doUpdate)
        setUpdatesEnabled(true);
}

GroupWidget * ParameterView::getGroupWidget_(const Parameter * p)
{
    auto i = groups_.find(p->groupId());
    if (i == groups_.end())
    {
        // create new
        GroupWidget * g = new GroupWidget(p->groupName(), container_);
        g->setMinimumWidth(300);

        layout_->addWidget(g);
        groups_.insert(p->groupId(), g);

        // get expanded flag from scene-settings
        MO_ASSERT(p->object(), "parameter without object in ParameterView");
        g->setExpanded(
            p->object()->getAttachedData(Object::DT_PARAM_GROUP_EXPANDED, p->groupId()).toBool()
                    );

        connect(g, &GroupWidget::expanded, [=]()
        {
            p->object()->setAttachedData(true, Object::DT_PARAM_GROUP_EXPANDED, p->groupId());
        });
        connect(g, &GroupWidget::collapsed, [=]()
        {
            p->object()->setAttachedData(false, Object::DT_PARAM_GROUP_EXPANDED, p->groupId());
            //squeezeView_();
        });

        return g;
    }
    // return existing
    return i.value();
}

void ParameterView::createWidgets_()
{
    setUpdatesEnabled(false);

    clearWidgets_();

    if (!object_)
        return;


    QWidget * prev = 0;
    for (auto p : parameters_)
    {
        // visibility
        if (p->isZombie())
            continue;

        auto w = new ParameterWidget(p, container_);
        paramMap_.insert(p, w);

        connect(w, SIGNAL(objectSelected(MO::Object*)),
                this, SIGNAL(objectSelected(MO::Object*)));
        connect(w, SIGNAL(statusTipChanged(QString)),
                this, SIGNAL(statusTipChanged(QString)));

        if (!p->groupId().isEmpty())
        {
            GroupWidget * group = getGroupWidget_(p);
            group->addWidget(w);
            group->setVisible(w, p->isVisible());
        }
        else
        {
            if (!p->isVisible())
                w->setVisible(false);
            layout_->addWidget(w);
            widgets_.append(w);
        }

        if (prev)
            setTabOrder(prev, w);
        prev = w;
    }

    // a "stretch" that can be deleted later
    p_stretch_ = new QWidget(container_);
    layout_->addWidget(p_stretch_);
    layout_->setStretch(layout_->indexOf(p_stretch_), 2);
    layout_->activate();

    // another hack... duh
    container_->setGeometry(
                0, 0, scrollArea_->width()
                        -scrollArea_->verticalScrollBar()->width()-2,
                container_->height());

    //squeezeView_();

    // restore scrollbar position
    if (object_->hasAttachedData(Object::DT_PARAM_VIEW_Y))
    {
        //MO_PRINT("restore " << object_->getAttachedData(Object::DT_PARAM_VIEW_Y).toInt());
        scrollArea_->verticalScrollBar()->setValue(
                    object_->getAttachedData(Object::DT_PARAM_VIEW_Y).toInt());
    }
    else
    {
        if (scrollArea_->widget()->focusWidget())
            scrollArea_->ensureWidgetVisible(scrollArea_->widget()->focusWidget());
        else
            scrollArea_->verticalScrollBar()->setValue(0);
    }

    setUpdatesEnabled(true);
    doStoreScrollbarPos_ = true;
}



void ParameterView::onSequenceChanged(Sequence * seq)
{
    if (seq == object_)
        updateWidgetValues_();
}

void ParameterView::updateWidgetValues_()
{
    for (auto i = paramMap_.begin(); i != paramMap_.end(); ++i)
    {
        i.value()->updateWidgetValue();
    }
}

void ParameterView::updateWidgetValue_(Parameter * p)
{
    auto i = paramMap_.find(p);
    if (i != paramMap_.end())
        i.value()->updateWidgetValue();
}

void ParameterView::updateParameters()
{
    if (object_)
    {
        clearWidgets_();
        parameters_ = object_->params()->parameters();
        createWidgets_();
    }
}

void ParameterView::onScrollBarChange_(int v)
{
    //MO_PRINT(v);
    if (doStoreScrollbarPos_ && object_)
        object_->setAttachedData(v, Object::DT_PARAM_VIEW_Y);
}

} // namespace GUI
} // namespace MO
