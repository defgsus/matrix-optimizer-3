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
#include "util/objectmenu.h"
#include "model/objecttreemodel.h"

namespace MO {
namespace GUI {



ClipView::ClipView(QWidget * parent)
    : QWidget   (parent),
      clipCon_  (0),
      curNumX_ (0),
      curNumY_ (0),
      curX_     (0),
      curY_     (0),
      selStartX_(0),
      selStartY_(0),
      curClip_  (0)
{
    setObjectName("_ClipView");

    setMinimumSize(240, 120);

    // background color
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(20,20,20));
    setPalette(p);
    setAutoFillBackground(true);


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
    if (con && clipCon_ == con)
    {
        // don't update if all is same
        if (curNumX_ == con->numberColumns()
            && curNumY_ == con->numberRows())
            return;
    }

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
    {
        curNumX_ = curNumY_ = 0;
        return;
    }

    curNumX_ = clipCon_->numberColumns();
    curNumY_ = clipCon_->numberRows();

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

void ClipView::updateClip(Clip * clip)
{
    if (!clip || !clipCon_ || clip->clipContainer() != clipCon_)
        return;

    updateClipWidget_(clip->column(), clip->row());
}

void ClipView::updateClip(uint x, uint y)
{
    updateClipWidget_(x, y);
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

    if (w && clip != w->clip())
        w->setClip(clip);
}

void ClipView::removeObject(const Object *o)
{
    if (o == clipCon_)
    {
        setClipContainer(0);
        return;
    }

    // see if o was a clip
    auto w = widgetForClip_(static_cast<const Clip*>(o));
    if (w)
    {
        w->setClip(0);
    }
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

ClipWidget * ClipView::widgetForClip_(const Clip * c)
{
    auto i = widgetMap_.find(c);
    return (i == widgetMap_.end())? 0 : i.value();
}

void ClipView::onClicked_(ClipWidget * w, Qt::MouseButtons b, Qt::KeyboardModifiers mod)
{
    // set current selection
    curClip_ = w->clip();
    curX_ = w->posX();
    curY_ = w->posY();

    // left-click
    if (b & Qt::LeftButton)
    {
        // add-sub select
        if (mod & Qt::ControlModifier)
        {
            selection_.flip(w);
            w->update();
        }
        // range select
        else if (mod & Qt::ShiftModifier)
        {
            const uint
                    mix = std::min(selStartX_, curX_),
                    max = std::max(selStartX_, curX_),
                    miy = std::min(selStartY_, curY_),
                    may = std::max(selStartY_, curY_);
            clearSelection_();
            for (uint y=miy; y<=may; ++y)
            for (uint x=mix; x<=max; ++x)
            {
                select_(clipWidget_(x, y));
            }
        }
        // simple select
        else
        {
            if (!isSelected(w))
            {
                clearSelection_();
                select_(w);
                selStartX_ = curX_;
                selStartY_ = curY_;
            }
        }

    }
    // right-click
    else if (b & Qt::RightButton)
    {
        openPopup_();
    }
}

void ClipView::clearSelection_()
{
    for (auto w : selection_)
        w->update();
    selection_.clear();
}

void ClipView::select_(ClipWidget * w)
{
    MO_ASSERT(w, "ClipView::select_(NULL) called");

    selection_.select(w);
    w->update();
}

void ClipView::openPopup_()
{
    if (!clipCon_)
        return;

    Scene * scene = clipCon_->sceneObject();
    if (!scene)
    {
        MO_WARNING("ClipView: Can't edit without Scene object");
        return;
    }

    ObjectTreeModel * model = scene->model();
    if (!model)
    {
        MO_WARNING("ClipView: Can't edit without ObjectTreeModel");
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
            clip->setPosition(curX_, curY_);
            if (!model->addObject(clipCon_, clip))
                delete clip;
        });
    }

    // ---- clip actions ---

    if (curClip_)
    {
        // add new sequence
        menu->addAction(a = new QAction(tr("Add object"), menu));
        QMenu * sub = ObjectMenu::createObjectMenu(Object::TG_SEQUENCE, menu);
        a->setMenu(sub);
        connect(sub, &QMenu::triggered, [=](QAction * a)
        {
            Object * o = ObjectFactory::createObject(a->data().toString());
            MO_ASSERT(o, "ClipView: Could not create object class '" << a->data().toString() << "'");
            if (!model->addObject(curClip_, o))
                delete o;
        });


        menu->addSeparator();

        // delete clip
        menu->addAction(a = new QAction(tr("Delete clip"), menu));
        connect(a, &QAction::triggered, [=]()
        {
            const QModelIndex idx = model->indexForObject(curClip_);
            model->deleteObject(idx);
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
