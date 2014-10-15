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
#include <QClipboard>

#include "clipview.h"
#include "widget/clipwidget.h"
#include "object/clipcontainer.h"
#include "object/clip.h"
#include "object/scene.h"
#include "object/objectfactory.h"
#include "io/error.h"
#include "io/log.h"
#include "io/application.h"
#include "util/objectmenu.h"
#include "model/objecttreemodel.h"
#include "model/objecttreemimedata.h"

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
      curClip_  (0),
      dragWidget_(0),
      goalWidget_(0)
{
    setObjectName("_ClipView");

    setMinimumSize(240, 120);

    // background color
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(20,20,20));
    setPalette(p);
    setAutoFillBackground(true);


    createWidgets_();
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
        scrollAreaV_->setMaximumWidth(ClipWidget::sizeForType(ClipWidget::T_ROW).width() + 2);

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
        scrollAreaH_->setMaximumHeight(ClipWidget::sizeForType(ClipWidget::T_COLUMN).height() + 2);

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

    // disconnect previous
    if (clipCon_)
    {
        disconnect(clipCon_, SIGNAL(clipTriggered(Clip*)), this, SLOT(onClipTriggered_(Clip*)));
        disconnect(clipCon_, SIGNAL(clipStopTriggered(Clip*)), this, SLOT(onClipStopTriggered_(Clip*)));
        disconnect(clipCon_, SIGNAL(clipStarted(Clip*)), this, SLOT(onClipStarted_(Clip*)));
        disconnect(clipCon_, SIGNAL(clipStopped(Clip*)), this, SLOT(onClipStopped_(Clip*)));
    }

    clipCon_ = con;

    // connect
    connect(clipCon_, SIGNAL(clipTriggered(Clip*)), this, SLOT(onClipTriggered_(Clip*)));
    connect(clipCon_, SIGNAL(clipStopTriggered(Clip*)), this, SLOT(onClipStopTriggered_(Clip*)));
    connect(clipCon_, SIGNAL(clipStarted(Clip*)), this, SLOT(onClipStarted_(Clip*)));
    connect(clipCon_, SIGNAL(clipStopped(Clip*)), this, SLOT(onClipStopped_(Clip*)));

    createClipWidgets_();
}

void ClipView::createClipWidgets_()
{
    // clear everything that has pointers to objects
    widgetMap_.clear();
    selection_.clear();
    goalSelection_.clear();
    curClip_ = 0;
    dragWidget_ = goalWidget_ = 0;

    // remove previous widgets
    for (auto w : clipWidgets_)
    {
        w->setVisible(false); // save layout from additional work
        w->deleteLater();
    }
    clipWidgets_.clear();

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
        connect(w, SIGNAL(buttonClicked(ClipWidget*)),
                this, SLOT(onButtonClicked_(ClipWidget*)));
    }

    // row headers
    for (uint y=0; y<clipCon_->numberRows(); ++y)
    {
        auto w = new ClipWidget(ClipWidget::T_ROW, 0, this);
        w->setPos(0, y);
        w->setName(clipCon_->rowName(y));
        layoutV_->addWidget(w, y, 0);
        clipWidgets_.append(w);
        connect(w, SIGNAL(buttonClicked(ClipWidget*)),
                this, SLOT(onButtonClicked_(ClipWidget*)));
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
        // connect clipwidget
        connect(w, SIGNAL(clicked(ClipWidget*,Qt::MouseButtons,Qt::KeyboardModifiers)),
                this, SLOT(onClicked_(ClipWidget*,Qt::MouseButtons,Qt::KeyboardModifiers)));
        connect(w, SIGNAL(moved(ClipWidget*,QPoint,Qt::MouseButtons,Qt::KeyboardModifiers)),
                this, SLOT(onMoved_(ClipWidget*,QPoint,Qt::MouseButtons,Qt::KeyboardModifiers)));
        connect(w, SIGNAL(released(ClipWidget*,Qt::MouseButtons,Qt::KeyboardModifiers)),
                this, SLOT(onReleased_(ClipWidget*,Qt::MouseButtons,Qt::KeyboardModifiers)));
        connect(w, SIGNAL(buttonClicked(ClipWidget*)),
                this, SLOT(onButtonClicked_(ClipWidget*)));
    }
}

bool ClipView::isSelected(ClipWidget * c) const
{
    return selection_.isSelected(c)
        || goalSelection_.isSelected(c);
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

    if (!w)
    {
        MO_WARNING("ClipView::updateClipWidget_(" << x << ", " << y << ") "
                   "no ClipWidget at this position");
        return;
    }

    // update clip in clipwidget
    if (clip != w->clip())
        w->setClip(clip);

    if (clip->color() != w->clipColor())
        w->setClipColor(clip->color());

    // update widgetmap
    if (clip && !widgetMap_.contains(clip))
        widgetMap_.insert(clip, w);
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
    if (!clipCon_)
        return;

    // set current selection
    curClip_ = w->clip();
    curX_ = w->posX();
    curY_ = w->posY();

    // left-click
    if (b & Qt::LeftButton)
    {
        clickSelect_(w, mod);

        dragWidget_ = w;
    }
    // right-click
    else if (b & Qt::RightButton)
    {
        clickSelect_(w, mod);

        openPopup_();
    }
}

void ClipView::clickSelect_(ClipWidget *w, Qt::KeyboardModifiers mod)
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
        if (!selection_.isSelected(w))
        {
            clearSelection_();
            select_(w);
            selStartX_ = curX_;
            selStartY_ = curY_;
        }
    }
}

void ClipView::onMoved_(ClipWidget * widget, const QPoint &wpos, Qt::MouseButtons b, Qt::KeyboardModifiers mod)
{
    if (!clipCon_ || !dragWidget_ || selection_.isEmpty() || !(b & Qt::LeftButton))
        return;

    // find move-to position
    QPoint pos = widget->mapTo(this, wpos);
    ClipWidget * goalw = qobject_cast<ClipWidget*>(childAt(pos));
    if (!goalw || goalw->type() != ClipWidget::T_CLIP)
        return;

    int dx = (int)goalw->posX() - (int)dragWidget_->posX(),
        dy = (int)goalw->posY() - (int)dragWidget_->posY();

    goalWidget_ = (dx != 0 || dy != 0)? goalw : 0;

    // update goal selection
    for (auto w : goalSelection_)
        w->update();
    goalSelection_.clear();

    // goal-select each selected clip
    for (auto w : selection_)
    {
        if (!w->clip())
            continue;

        uint newx = w->posX() + dx,
             newy = w->posY() + dy;

        if (newx >= clipCon_->numberColumns()
         || newy >= clipCon_->numberRows())
            continue;

        ClipWidget * goal = clipWidget_(newx, newy);
        if (!goal)
            continue;
        goalSelection_.select(goal);
        goal->update();
    }
}

void ClipView::onReleased_(ClipWidget * w, Qt::MouseButtons, Qt::KeyboardModifiers )
{
    if (!clipCon_ || !dragWidget_ || (w != dragWidget_) || !goalWidget_)
        return;

    // find move-to position
    int dx = (int)goalWidget_->posX() - dragWidget_->posX(),
        dy = (int)goalWidget_->posY() - dragWidget_->posY();

    // clear drag state
    dragWidget_ = goalWidget_ = 0;

    // clear goal selection
    for (auto w : goalSelection_)
        w->update();
    goalSelection_.clear();

    if (dx != 0 || dy != 0)
        moveSelection_(dx, dy);
}

void ClipView::onButtonClicked_(ClipWidget * w)
{
    if (!clipCon_)
        return;

    // get the current scene time
    Double gtime = 0;
    if (Scene * scene = clipCon_->sceneObject())
    {
        gtime = scene->sceneTime();
    }

    switch (w->type())
    {
        case ClipWidget::T_CLIP:
            if (w->clip())
                clipCon_->triggerClip(w->clip(), gtime);
        break;

        case ClipWidget::T_ROW:
            clipCon_->triggerRow(w->posY(), gtime);
        break;

        case ClipWidget::T_COLUMN:
            clipCon_->triggerStopColumn(w->posX(), gtime);
        break;
    }
}

void ClipView::onClipTriggered_(Clip * clip)
{
    if (ClipWidget * w = widgetForClip_(clip))
        w->setTriggered();
}

void ClipView::onClipStopTriggered_(Clip * clip)
{
    if (ClipWidget * w = widgetForClip_(clip))
        w->setStopTriggered();
}

void ClipView::onClipStarted_(Clip * clip)
{
    if (ClipWidget * w = widgetForClip_(clip))
        w->setStarted();
}

void ClipView::onClipStopped_(Clip * clip)
{
    if (ClipWidget * w = widgetForClip_(clip))
        w->setStopped();
}

void ClipView::moveSelection_(int dx, int dy)
{
    // temporary struct to remember the moves
    // because clips might temporarily be placed on one another
    struct Move
    {
        ClipWidget *from, *to;
        Clip * clip;
    };
    QVector<Move> moves;

    // move each clip
    for (auto w : selection_)
    {
        if (!w->clip())
            continue;

        uint x = w->clip()->column(),
             y = w->clip()->row(),
             newx = x + dx,
             newy = y + dy;

        if (newx >= clipCon_->numberColumns()
         || newy >= clipCon_->numberRows())
            continue;

//        MO_DEBUG("move " << x << ", " << y << " to "
//                 << newx << ", " << newy);

        // goal clipwidget
        ClipWidget * tow = clipWidget_(newx, newy);

        // don't move on top of another clip
        // unless it also gets moved
        if (tow->clip() && !selection_.isSelected(tow))
            continue;

        // remember the move
        Move m;
        m.from = w;
        m.to = tow;
        m.clip = w->clip();
        moves.append(m);
    }

    clearSelection_();

    // execute moves
    for (auto & m : moves)
    {
//        MO_DEBUG("move " << m.from->posX() << ", " << m.from->posY()
//                 << m.to->posX() << ", " << m.to->posY());

        // set clip position
        m.clip->setPosition(m.to->posX(), m.to->posY());
        // update clipwidgets
        if (!selection_.isSelected(m.from))
            m.from->setClip(0);
        m.to->setClip(m.clip);
        // update widget map
        widgetMap_.remove(m.clip);
        widgetMap_.insert(m.clip, m.to);
        // select
        selection_.select(m.to);
    }

    clipCon_->updateClipPositions();
}


void ClipView::clearSelection_()
{
    for (auto w : selection_)
        w->update();
    for (auto w : goalSelection_)
        w->update();
    selection_.clear();
    goalSelection_.clear();
}

void ClipView::select_(ClipWidget * w)
{
    MO_ASSERT(w, "ClipView::select_(NULL) called");

    selection_.select(w);
    w->update();
}

void ClipView::moveClip_(ClipWidget * w, uint newx, uint newy)
{
    if (!clipCon_)
        return;

    if (newx >= clipCon_->numberColumns()
        || newy >= clipCon_->numberRows())
    {
        MO_WARNING("ClipView::moveClip_(" << w << ", " << newx << ", " << newy << ") "
                   "out of range (" << clipCon_->numberColumns() << ", " << clipCon_->numberRows() << ")");
        return;
    }

    // don't need to move an empty clipwidget
    if (!w->clip())
        return;

    Clip * clip = w->clip();

    //uint oldx = clip->column(),
    //     oldy = clip->row();
    // move clip
    clip->setPosition(newx, newy);
    // update clip container
    clipCon_->updateClipPositions();
    // update widgetmap
    ClipWidget * neww = clipWidget_(newx, newy);
    widgetMap_.remove(clip);
    widgetMap_.insert(clip, neww);
    // update clipwidgets
    w->setClip(0);
    neww->setClip(clip);
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

    ClipWidget * curWidget = widgetForClip_(curClip_);

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

        // from clipboard actions
        if (ObjectTreeMimeData::isObjectTypeInClipboard(Object::T_CLIP))
        {
            const bool plural = ObjectTreeMimeData::numObjectsInClipboard() > 1;

            // paste clip
            menu->addAction(a = new QAction(plural ? tr("Paste clips") : tr("Paste clip"), menu));
            a->setShortcut(Qt::CTRL + Qt::Key_V);
            connect(a, &QAction::triggered, [=]()
            {
                QList<Object*> list =
                static_cast<const ObjectTreeMimeData*>(
                        application->clipboard()->mimeData())->getObjectTrees();
                if (list.empty())
                    return;
                pasteClips_(list, curX_, curY_);
            });
        }
    }

    // ---- clip actions ---

    if (curClip_)
    {
        // add new sequence
        menu->addAction(a = new QAction(tr("Add object to clip"), menu));
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

        // set color
        menu->addAction(a = new QAction(tr("Change clip color"), menu));
        sub = ObjectMenu::createColorMenu(menu);
        a->setMenu(sub);
        connect(sub, &QMenu::triggered, [=](QAction * a)
        {
            QColor c = a->data().value<QColor>();
            curClip_->setColor(c);
            updateClip(curClip_);
        });

        menu->addSeparator();

        // single clip actions
        if (selection_.size() < 2)
        {
            // copy clip
            menu->addAction(a = new QAction(tr("Copy clip"), menu));
            a->setShortcut(Qt::CTRL + Qt::Key_C);
            connect(a, &QAction::triggered, [=]()
            {
                auto data = new ObjectTreeMimeData();
                data->storeObjectTree(curClip_);
                application->clipboard()->setMimeData(data);
            });

            // cut clip
            menu->addAction(a = new QAction(tr("Cut clip"), menu));
            a->setShortcut(Qt::CTRL + Qt::Key_X);
            connect(a, &QAction::triggered, [=]()
            {
                auto data = new ObjectTreeMimeData();
                data->storeObjectTree(curClip_);
                application->clipboard()->setMimeData(data);
                const QModelIndex idx = model->indexForObject(curClip_);
                model->deleteObject(idx);
                selection_.unselect(curWidget);
            });

            // delete clip
            menu->addAction(a = new QAction(tr("Delete clip"), menu));
            connect(a, &QAction::triggered, [=]()
            {
                const QModelIndex idx = model->indexForObject(curClip_);
                model->deleteObject(idx);
                selection_.unselect(curWidget);
            });
        }
        // multi clip actions
        else
        {
            // copy clips
            menu->addAction(a = new QAction(tr("Copy clips"), menu));
            a->setShortcut(Qt::CTRL + Qt::Key_C);
            connect(a, &QAction::triggered, [=]()
            {
                auto data = new ObjectTreeMimeData();
                QList<Object*> list;
                for (auto w : selection_)
                    if (w->clip())
                        list << w->clip();
                data->storeObjectTrees(list);
                application->clipboard()->setMimeData(data);
            });

            // cut clips
            menu->addAction(a = new QAction(tr("Cut clips"), menu));
            a->setShortcut(Qt::CTRL + Qt::Key_X);
            connect(a, &QAction::triggered, [=]()
            {
                auto data = new ObjectTreeMimeData();
                QList<Object*> list;
                for (auto w : selection_)
                    if (w->clip())
                        list << w->clip();
                data->storeObjectTrees(list);
                application->clipboard()->setMimeData(data);
                for (auto w : selection_)
                if (w->clip())
                {
                    const QModelIndex idx = model->indexForObject(w->clip());
                    model->deleteObject(idx);
                }
                clearSelection_();
            });

            // delete clip
            menu->addAction(a = new QAction(tr("Delete clip"), menu));
            connect(a, &QAction::triggered, [=]()
            {
                for (auto w : selection_)
                if (w->clip())
                {
                    const QModelIndex idx = model->indexForObject(w->clip());
                    model->deleteObject(idx);
                }
                clearSelection_();
            });
        }
    }

    if (menu->isEmpty())
    {
        menu->deleteLater();
        return;
    }

    menu->popup(QCursor::pos());

}

void ClipView::pasteClips_(const QList<Object*>& list, uint x, uint y)
{
    uint minx = -1, miny = -1; // very large

    // find all clips and the object list
    // and get minimum row and column
    QList<Clip*> clips;
    for (auto obj : list)
    {
        if (Clip * clip = qobject_cast<Clip*>(obj))
        {
            clips << clip;
            minx = std::min(minx, clip->column());
            miny = std::min(miny, clip->row());
        }
        else
            delete obj;
    }

    MO_ASSERT(clipCon_ && clipCon_->sceneObject() &&
              clipCon_->sceneObject()->model(), "");

    auto model = clipCon_->sceneObject()->model();

    // paste clips
    bool resized = false;
    for (auto & clip : clips)
    {
        uint col = x + clip->column() - minx,
             row = y + clip->row() - miny;
        bool lresized;
        clipCon_->findNextFreeSlot(col, row, true, &lresized);
        resized |= lresized;
        clip->setPosition(col, row);
        if (!model->addObject(clipCon_, clip))
        {
            delete clip;
            clip = 0;
        }
    }

    // update everything on resize
    if (resized)
        createClipWidgets_();

    // select
    for (auto clip : clips)
    if (clip)
    {
        ClipWidget * w = widgetForClip_(clip);
        if (w)
            select_(w);
    }
}




} // namespace GUI
} // namespace MO
