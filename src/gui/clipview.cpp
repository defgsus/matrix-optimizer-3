/** @file clipview.cpp

    @brief Clip object view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include <QLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QMenu>
#include <QAction>

#include "clipview.h"
#include "widget/clipwidget.h"
#include "object/clipcontainer.h"
#include "object/clip.h"
#include "object/scene.h"
#include "object/objectfactory.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {



ClipView::ClipView(QWidget * parent)
    : QWidget   (parent),
      clipCon_  (0),
      curX_     (0),
      curY_     (0),
      curClip_  (0)
{
    setObjectName("_ClipView");

    setMinimumSize(240, 120);

    /*
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(20,20,20));
    setPalette(p);
    setAutoFillBackground(true);
    */

    createWidgets_();

    auto con = new ClipContainer(this);
    con->setNumber(40,30);
    setClipContainer(con);
}

void ClipView::createWidgets_()
{
    auto lg = new QGridLayout(this);
    lg->setContentsMargins(0,0,0,0);
    lg->setSpacing(0);

        // row headers

        scrollAreaV_ = new QScrollArea(this);
        lg->addWidget(scrollAreaV_, 1, 0);
        scrollAreaV_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollAreaV_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollAreaV_->setMaximumWidth(24);

        containerV_ = new QWidget(scrollAreaV_);
        containerV_->setObjectName("_rowheader_container");
        containerV_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

            // basic layout
            layoutV_ = new QGridLayout(containerV_);
            layoutV_->setSizeConstraint(QLayout::SetMinAndMaxSize);
            layoutV_->setContentsMargins(1,1,1,1);
            layoutV_->setSpacing(0);

            scrollAreaV_->setWidget(containerV_);

        // column headers

        scrollAreaH_ = new QScrollArea(this);
        lg->addWidget(scrollAreaH_, 0, 1);
        scrollAreaH_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollAreaH_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollAreaH_->setMaximumHeight(30);

        containerH_ = new QWidget(scrollAreaH_);
        containerH_->setObjectName("_columnheader_container");
        containerH_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

            // basic layout
            layoutH_ = new QGridLayout(containerH_);
            layoutH_->setSizeConstraint(QLayout::SetMinAndMaxSize);
            layoutH_->setContentsMargins(1,1,1,1);
            layoutH_->setSpacing(0);

            scrollAreaH_->setWidget(containerH_);

        // clips

        scrollArea_ = new QScrollArea(this);
        lg->addWidget(scrollArea_, 1, 1);

        container_ = new QWidget(scrollArea_);
        container_->setObjectName("_cliplayout_container");
        container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            // basic layout
            layout_ = new QGridLayout(container_);
            layout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
            layout_->setContentsMargins(1,1,1,1);
            layout_->setSpacing(0);

            layout_->setRowStretch(1000,1);
            layout_->setColumnStretch(1000,1);

            scrollArea_->setWidget(container_);

        // connect scrollareas
        connect(scrollArea_->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                scrollAreaH_->horizontalScrollBar(), SLOT(setValue(int)));
        connect(scrollArea_->horizontalScrollBar(), SIGNAL(rangeChanged(int,int)),
                scrollAreaH_->horizontalScrollBar(), SLOT(setRange(int,int)));
        connect(scrollArea_->verticalScrollBar(), SIGNAL(valueChanged(int)),
                scrollAreaV_->verticalScrollBar(), SLOT(setValue(int)));
        connect(scrollArea_->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
                scrollAreaV_->verticalScrollBar(), SLOT(setRange(int,int)));

}

void ClipView::setClipContainer(ClipContainer * con)
{
    clipCon_ = con;

    createClipWidgets_();
}

void ClipView::createClipWidgets_()
{
    // remove previous
    for (auto w : clipWidgets_)
    {
        w->setVisible(false);
        w->deleteLater();
    }
    clipWidgets_.clear();
    widgetMap_.clear();

    if (!clipCon_)
        return;

    // column headers
    for (uint x=0; x<clipCon_->numberColumns(); ++x)
    {
        auto w = new ClipWidget(ClipWidget::T_COLUMN, 0, this);
        w->setPos(x, 0);
        w->setName(clipCon_->columnName(x));
        layoutH_->addWidget(w, 0, x);
        clipWidgets_.append(w);
    }

    // row headers
    for (uint y=0; y<clipCon_->numberRows(); ++y)
    {
        auto w = new ClipWidget(ClipWidget::T_ROW, 0, this);
        w->setPos(0, y);
        w->setName(clipCon_->rowName(y));
        layoutV_->addWidget(w, y, 0);
        clipWidgets_.append(w);
    }

    // clips
    for (uint y=0; y<clipCon_->numberRows(); ++y)
    for (uint x=0; x<clipCon_->numberColumns(); ++x)
    {
        auto w = new ClipWidget(ClipWidget::T_CLIP, clipCon_->clip(x, y), this);
        w->setPos(x, y);
        layout_->addWidget(w, y, x);
        clipWidgets_.append(w);
        widgetMap_.insert(clipCon_->clip(x, y), w);
        connect(w, SIGNAL(clicked(ClipWidget*,Qt::MouseButtons,Qt::KeyboardModifiers)),
                this, SLOT(onClicked_(ClipWidget*,Qt::MouseButtons,Qt::KeyboardModifiers)));
    }
}

void ClipView::updateClipWidget_(uint x, uint y)
{
    if (!clipCon_)
        return;

    if (x >= clipCon_->numberColumns() || y >= clipCon_->numberRows())
    {
        MO_WARNING("ClipView::updateClipWidget_(" << x << ", " << y << ") out of range "
                   "(" << clipCon_->numberColumns() << ", " << clipCon_->numberRows() << ")");
        return;
    }

    Clip * clip = clipCon_->clip(x, y);
    ClipWidget * w = clipWidget_(x, y);

    if (clip != w->clip())
        w->setClip(clip);
}

ClipWidget * ClipView::clipWidget_(uint x, uint y)
{
    if (!clipCon_)
        return 0;

    int i = y * clipCon_->numberColumns() + x
            // + headers
            + clipCon_->numberColumns() + clipCon_->numberRows();

    if (i >= clipWidgets_.size())
    {
        MO_WARNING("ClipView::clipWidget_(" << x << ", " << y << ") out of range "
                   "(" << clipCon_->numberColumns() << ", " << clipCon_->numberRows() << ")");
        return 0;
    }

    return clipWidgets_[i];
}

ClipWidget * ClipView::widgetForClip_(Clip * c)
{
    auto i = widgetMap_.find(c);
    return (i == widgetMap_.end())? 0 : i.value();
}

void ClipView::onClicked_(ClipWidget * w, Qt::MouseButtons b, Qt::KeyboardModifiers)
{
    // set current selection
    curClip_ = w->clip();
    curX_ = w->posX();
    curY_ = w->posY();

    // pass further
    if (b & Qt::LeftButton)
        w->setSelected(true);
    else if (b & Qt::RightButton)
        openPopup_();
}


void ClipView::openPopup_()
{
    if (!clipCon_)
        return;

    Scene * scene = clipCon_->sceneObject();
    if (!scene)
    {
        MO_WARNING("Can't edit without Scene object");
        return;
    }

    QMenu * menu = new QMenu(this);
    connect(menu, SIGNAL(triggered(QAction*)), menu, SLOT(deleteLater()));

    QAction * a;

    // --- empty ---

    if (!curClip_)
    {
        // new clip
        menu->addAction(a = new QAction(tr("Create new clip"), menu));
        connect(a, &QAction::triggered, [=]()
        {
            Clip * clip = static_cast<Clip*>(ObjectFactory::createObject("Clip"));
            MO_DEBUG("set " << curX_ << ", " << curY_);
            clip->setPosition(curX_, curY_);
            scene->addObject(clipCon_, clip);
            updateClipWidget_(curX_, curY_);
        });
    }


    if (menu->isEmpty())
    {
        menu->deleteLater();
        return;
    }

    menu->popup(QCursor::pos());

}

} // namespace GUI
} // namespace MO
