/** @file trackview.cpp

    @brief Track view / Sequencer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/


#include <QDebug>

#include <QPalette>
#include <QMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QClipboard>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QAction>

#include "trackview.h"
#include "trackviewoverpaint.h"
#include "trackheader.h"
#include "widget/sequencewidget.h"
#include "object/control/sequencefloat.h"
#include "object/control/trackfloat.h"
#include "object/scene.h"
#include "object/util/objectfilter.h"
#include "object/util/objecteditor.h"
#include "object/objectfactory.h"
#include "model/objecttreemimedata.h"
#include "io/error.h"
#include "io/log.h"
#include "io/application.h"
#include "tool/enumnames.h"
#include "util/scenesettings.h"
#include "util/objectmenu.h"

namespace MO {
namespace GUI {

TrackView::TrackView(QWidget *parent) :
    QWidget         (parent),
    scene_          (0),
    objectFilter_   (new ObjectFilter()),
    currentObject_  (0),
    header_         (0),

    offsetY_                (0),
    maxHeight_              (0),

    action_                 (A_NOTHING_),
    selTrack_               (0),
    nextFocusSequence_      (0),
    currentTime_            (0),
    hoverWidget_            (0),

    filterCurrentObjectOnly_(true),
    filterAddModulatingObjects_(true),
    alwaysFullObject_       (true),

    trackSpacing_           (2),
    modifierMultiSelect_    (Qt::CTRL),
    modifierDragWithOffset_ (Qt::CTRL),
    selectSequenceOnSingleClick_(true)
{
    setObjectName("_TrackView");

    setStatusTip(tr("Track View: right-click for context menu / "
                    "left-click for selection frame "
                    "(%1 to flip selection)").arg(enumName(modifierMultiSelect_)));

    statusSeqNormal =
            tr("Sequence: right-click for context menu / "
               "left-click + drag to move / "
               "drag edges to change length, "
               "hold %1 to keep contents in place")
            .arg(enumName(modifierDragWithOffset_));
    // XXX does not update
    statusSeqLeftEdge =
            tr("drag to change start, hold %1 to keep contents in place")
                        .arg(enumName(modifierDragWithOffset_));

    setMinimumSize(320,40);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(50,50,50));
    setPalette(p);
    setAutoFillBackground(true);

    header_ = new TrackHeader(this, this);
    overpaint_ = new TrackViewOverpaint(this, this);

    penSelectFrame_ = QPen(Qt::white);
    penSelectFrame_.setStyle(Qt::DashLine);
    penFramedWidget_ = QPen(QColor(200,200,200));
    penFramedWidget_.setStyle(Qt::DotLine);
    penCurrentTime_ = QPen(QColor(255,255,255,60));
}

TrackView::~TrackView()
{
    delete objectFilter_;
}

void TrackView::setViewSpace(const UTIL::ViewSpace & s)
{
    space_ = s;

    updateWidgetsViewSpace_();
}

void TrackView::setVerticalOffset(int y)
{
    bool changed = (offsetY_ != y);
    offsetY_ = y;
    if (changed)
    {
        updateWidgetsViewSpace_();
        header_->setVerticalOffset(offsetY_);
        update();
    }
}

QPointF TrackView::screenToView(const QPoint &screen) const
{
    return QPointF(
                space_.mapXTo((Double)screen.x() / width()),
                screen.y() + offsetY_);
}

QPoint TrackView::viewToScreen(const QPointF &view) const
{
    return QPoint(
                space_.mapXFrom(view.x()) * width(),
                view.y() - offsetY_);
}

void TrackView::resizeEvent(QResizeEvent *)
{
    overpaint_->setGeometry(rect());
    updateWidgetsViewSpace_();

    // XXX need to focus (scrollbar) on selected sequences!!
    if (!selectedWidgets_.isEmpty())
    {
        const int y = trackHeight(selectedWidgets_[0]->track());
        emit scrollTo(y);
    }
}

void TrackView::paintEvent(QPaintEvent * )
{
    QPainter p(this);

    // background
    p.setPen(QColor(70,70,70));
    for (auto t : tracks_)
    {
        const int y = trackY(t) + trackHeight(t) + trackSpacing_ / 2;
        p.drawLine(0, y, width(), y);
    }
}

SequenceWidget * TrackView::widgetForSequence_(Sequence * seq) const
{
    for (auto s : sequenceWidgets_)
        if (s->sequence() == seq)
            return s;
    return 0;
}

void TrackView::updateWidgetsViewSpace_()
{
    setCurrentTime_(currentTime_);

    for (auto s : sequenceWidgets_)
    {
        updateWidgetViewSpace_(s);
    }
}

bool TrackView::updateWidgetViewSpace_(SequenceWidget * s)
{
    const int h = trackHeight(s->track()),
              y = trackY(s->track());
    QRect r(0, y, 10, h);

    r.setLeft(space_.mapXFrom(s->sequence()->start()) * width());
    r.setRight(space_.mapXFrom(s->sequence()->end()) * width());
    if (r != s->geometry())
    {
        s->setGeometry(r);
        s->updateViewSpace();
        return true;
    }
    return false;
}

void TrackView::clearTracks()
{
    clearTracks_(false);
}

void TrackView::clearTracks_(bool keep_alltracks)
{
    if (!keep_alltracks)
        allObjects_.clear();
    tracks_.clear();
    trackY_.clear();

    for (auto s : sequenceWidgets_)
        s->deleteLater();

    sequenceWidgets_.clear();
    selectedWidgets_.clear();
    framedWidgets_.clear();

    offsetY_ = 0;

    header_->clearTracks();
}


void TrackView::setCurrentObject(Object * obj, bool send_signal)
{
//    MO_ASSERT(sceneSettings_, "TrackView::setObjects() without SceneSettings");

//    if (obj == currentObject_)
//        return;

    // remove previous content
    clearTracks();

    if (!obj)
    {
        currentObject_ = 0;
        return;
    }

    // determine scene
    Scene * scene = obj->sceneObject();
    MO_ASSERT(scene, "Scene not set in TrackView::setCurrentObject()");

    if (scene != scene_)
    {
        editor_ = scene->editor();
        MO_ASSERT(editor_, "Scene in TrackView without editor");

        connect(editor_, SIGNAL(objectChanged(MO::Object*)),
                this, SLOT(onObjectChanged_(MO::Object*)));
        connect(editor_, SIGNAL(sequenceChanged(MO::Sequence*)),
                this, SLOT(onSequenceChanged_(MO::Sequence*)));
        connect(editor_, SIGNAL(parameterChanged(MO::Parameter*)),
                this, SLOT(onParameterChanged_(MO::Parameter*)));
        //connect(editor_, &ObjectEditor::objectAdded)
    }
    scene_ = scene;
    currentObject_ = obj;


    // filter visible tracks
    getFilteredTracks_(tracks_);

    calcTrackY_();

    for (auto t : tracks_)
        createSequenceWidgets_(t);

    assignModulatingWidgets_();
    updateWidgetsViewSpace_();
    update();

    header_->setTracks(tracks_);

    if (send_signal)
        emit tracksChanged();
}


void TrackView::getFilteredTracks_(QList<Track *> &list)
{
    if (filterCurrentObjectOnly_ && currentObject_) // belong-to-object
    {
        Object * o = getContainerObject_(currentObject_);

        QList<Object*> list = o->findChildObjects(Object::TG_ALL, true);
        list.prepend(o);

        allObjects_ = list;

        if (filterAddModulatingObjects_)
        {
            allObjects_ << o->getModulatingObjectsList(true);
        }

    }
    else
    {
        // copy tracks
        allObjects_ = scene_->findChildObjects(Object::TG_ALL, true);
        // expand by modulators
        //objectFilter_->addAllModulators(obj, allObjects_);
    }

    // setup filter
    //objectFilter_->setBelongToFilter(currentObject_);
    //objectFilter_->setExpandObjects(true);

    // transform
    list.clear();
    objectFilter_->transform(allObjects_, list);

    qStableSort(list.begin(), list.end(), [=](Track * r, Track * l)
    {
        //if (r->parentObject() && l->parentObject())
        //  return r->parentObject()->name() < l->parentObject()->name();
        return r->namePath() < l->namePath();
        //return r->name() < l->name();
    });
}

Object * TrackView::getContainerObject_(Object * o)
{
    int mask = Object::TG_REAL_OBJECT;

    // at least the track
    if (!alwaysFullObject_)
        mask |= Object::TG_TRACK;

    while (!(o->type() & mask))
    {
        if (!o->parentObject())
            return o;

        o = o->parentObject();
    }
    return o;
}


QMenu * TrackView::createFilterMenu()
{
    QMenu * m = new QMenu(this);
    QAction * a;

    QAction * curA = a = new QAction(tr("only show current object"), m);
    m->addAction(a);
    a->setCheckable(true);
    a->setChecked(filterCurrentObjectOnly_);

    QAction * fullA = a = new QAction(tr("always show full object"), m);
    m->addAction(a);
    a->setCheckable(true);
    a->setChecked(alwaysFullObject_);
    a->setEnabled(filterCurrentObjectOnly_);
    connect(a, &QAction::triggered, [=]()
    {
        alwaysFullObject_ = a->isChecked();
        setCurrentObject(currentObject_);
    });

    QAction * modA = a = new QAction(tr("include modulators"), m);
    m->addAction(a);
    a->setCheckable(true);
    a->setChecked(filterAddModulatingObjects_);
    a->setEnabled(filterCurrentObjectOnly_);
    connect(a, &QAction::triggered, [=]()
    {
        filterAddModulatingObjects_ = a->isChecked();
        setCurrentObject(currentObject_);
    });

    connect(curA, &QAction::triggered, [=]()
    {
        filterCurrentObjectOnly_ = curA->isChecked();
        fullA->setEnabled(filterCurrentObjectOnly_);
        modA->setEnabled(filterCurrentObjectOnly_);
        setCurrentObject(currentObject_);
    });

    return m;
}


void TrackView::calcTrackY_()
{
    // set track positions
    int y = 0;
    for (auto t : tracks_)
    {
        const int h = trackHeight(t);
        trackY_.insert(t, y);
        y += h + trackSpacing_;
    }

    maxHeight_ = y;
}


int TrackView::trackHeight(Track * t) const
{
    return t->hasAttachedData(Object::DT_TRACK_HEIGHT)
            ? t->getAttachedData(Object::DT_TRACK_HEIGHT).toInt()
            : 32;
}

void TrackView::setTrackHeight(Track * t, int h)
{
    t->setAttachedData(h, Object::DT_TRACK_HEIGHT);

    calcTrackY_();
    updateWidgetsViewSpace_();
    update();
    emit tracksChanged();
}

int TrackView::trackY(Track * t) const
{
    return trackY_.value(t, 0) - offsetY_;
}

void TrackView::createSequenceWidgets_(Track * t)
{
    MO_DEBUG_GUI("TrackView::createSequenceWidgets(" << t << ")");

    MO_ASSERT(t, "TrackView::createSequenceWidgets() with NULL Track");

    if (!trackY_.contains(t))
    {
        MO_WARNING("TrackView::createSequenceWidgets() for unknown Track '" << t->idName() << "' requested.");
        return;
    }

    // delete the previous widgets for this track
    decltype(sequenceWidgets_) newlist;
    for (auto w : sequenceWidgets_)
    {
        if (w->track() == t)
        {
            w->deleteLater();
            selectedWidgets_.removeOne(w);
        }
        else
            newlist.insert(w);
    }
    sequenceWidgets_ = newlist;
    hoverWidget_ = 0;

    // all sequences on track
    QList<Sequence*> seqs = t->findChildObjects<Sequence>();

    // create SequenceWidgets
    for (Sequence * seq : seqs)
    {
        MO_ASSERT(seq->parentTrack(), "No parent track for sequence in TrackView");

        // create the widget
        auto w = new SequenceWidget(t, seq, this);
        sequenceWidgets_.insert( w );

        w->setStatusTip(statusSeqNormal);

        // initialize
        w->setVisible(true);
        connect(w, SIGNAL(hovered(SequenceWidget*,bool)),
                this, SLOT(widgetHovered_(SequenceWidget*,bool)));

        // set the focus
        if (seq == nextFocusSequence_)
        {
            w->setFocus();
            nextFocusSequence_ = 0;
            clearSelection_();
            selectSequenceWidget_(w, SELECT_);
            emit sequenceSelected(seq);
        }
    }

    // needs to be on top
    overpaint_->raise();
}

void TrackView::assignModulatingWidgets_()
{
    for (auto w : sequenceWidgets_)
        w->influencedWidgets().clear();

    for (auto w : sequenceWidgets_)
    {
        // find objects that influence 'w'
        QList<Object*> mods = w->sequence()->getModulatingObjectsList(true);
        for (auto m : mods)
            // if it's a sequence ..
            if (auto seq = qobject_cast<Sequence*>(m))
                // .. and in this view ..
                if (auto sw = widgetForSequence_(seq))
                    // .. then tell it that 'w' needs an update when it changes
                    sw->influencedWidgets().append(w);
    }
}

void TrackView::updateTrack(Track * t)
{
    MO_ASSERT(t, "TrackView::updateTrack() with NULL Track");

    if (!trackY_.contains(t))
    {
        MO_WARNING("TrackView::updateTrack() for unknown Track '" << t->idName() << "' requested.");
        return;
    }

    createSequenceWidgets_(t);

    updateWidgetsViewSpace_();
}

void TrackView::setCurrentTime_(Double t)
{
    currentTime_ = t;

    const int
            w = penCurrentTime_.width(),
            x = space_.mapXFrom(currentTime_) * width();

    update(oldCurrentRect_);
    oldCurrentRect_ = QRect(x-w/2,0, w,height());
    update(oldCurrentRect_);
}

void TrackView::widgetHovered_(SequenceWidget * w, bool on)
{
    if (action_ == A_NOTHING_)
        hoverWidget_ = on? w : 0;
}

void TrackView::mouseDoubleClickEvent(QMouseEvent * e)
{
    // doubleclick on sequence
    if (!selectSequenceOnSingleClick_)
    if (hoverWidget_ && e->button() == Qt::LeftButton)
    {
        emit sequenceSelected(hoverWidget_->sequence());

        e->accept();
        return;
    }
}

void TrackView::mousePressEvent(QMouseEvent * e)
{
    bool multisel = (e->modifiers() & modifierMultiSelect_);

    // --- clicked on sequence ---

    // !! a popup eats the hoverWidget_
    QWidget * w = childAt(e->pos());
    if (SequenceWidget * sw = qobject_cast<SequenceWidget*>(w))
        hoverWidget_ = sw;

    if (hoverWidget_)
    {
        if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton)
        {
            // change select state

            if (multisel)
                selectSequenceWidget_(hoverWidget_, FLIP_);
            else
            {
                if (!hoverWidget_->selected())
                {
                    clearSelection_();
                    selectSequenceWidget_(hoverWidget_, SELECT_);
                }
            }

            // emit selected-signal
            if (selectSequenceOnSingleClick_)
            {
                emit sequenceSelected(hoverWidget_->sequence());
            }

            // leftclick on sequence
            if (e->button() == Qt::LeftButton)
            {
                if (isSelected_())
                {
                    dragStartTime_ = space_.mapXTo((Double)e->x()/width());
                    dragStartTrack_ = hoverWidget_->track();
                    dragStartTimes_.clear();
                    dragStartLengths_.clear();
                    for (auto w : selectedWidgets_)
                    {
                        dragStartTimes_.append(w->sequence()->start());
                        dragStartLengths_.append(w->sequence()->length());
                    }

                    // --- start drag right ---

                    if (hoverWidget_->onRightEdge())
                    {
                        action_ = A_DRAG_RIGHT_;
                    }
                    else

                    // --- start drag left ---

                    if (hoverWidget_->onLeftEdge())
                    {
                        action_ = A_DRAG_LEFT_;
                        dragStartOffsets_.clear();
                        for (auto w : selectedWidgets_)
                            dragStartOffsets_.append(w->sequence()->timeOffset());
                        hoverWidget_->setStatusTip(statusSeqLeftEdge);
                    }
                    else

                    // --- start drag pos ---

                    {
                        action_ = A_DRAG_POS_;
                    }
                }

                e->accept();
                return;
            }

            // --- right-click on sequence ---
            if (e->button() == Qt::RightButton)
            {
                createEditActions_();

                QMenu * popup = new QMenu(this);
                popup->addActions(editActions_);

                popup->popup(QCursor::pos());

                e->accept();
                return;
            }
        }
    }

    // --- clicked on track ---

    if (!multisel)
        clearSelection_();

    selTrack_ = trackForY_(e->y());
    setCurrentTime_( space_.mapXTo((Double)e->x() / width()) );

    if (selTrack_)
    {
        // right-click on track
        if (e->button() == Qt::RightButton)
        {
            createEditActions_();

            QMenu * popup = new QMenu(this);
            popup->addActions(editActions_);

            popup->popup(QCursor::pos());

            e->accept();
            return;
        }
    }

    // --- start selection frame ---

    if (e->button() == Qt::LeftButton)
    {
        action_ = A_SELECT_FRAME_;
        dragStartPosV_ = screenToView(e->pos());
        selectRect_ = QRect(dragStartPos_, QSize(0,0));
        framedWidgets_.clear();
    }
}

void TrackView::mouseMoveEvent(QMouseEvent * e)
{
    if (action_ == A_NOTHING_)
        return;

    Double timeX = space_.mapXTo((Double)e->x() / width()),
           deltaTime = timeX - dragStartTime_;

    // ---- drag sequence(s) position ----

    if (action_ == A_DRAG_POS_)
    {
        for (int i=0; i<selectedWidgets_.size(); ++i)
        {
            Double newstart = std::max((Double)0, dragStartTimes_[i] + deltaTime);
            Sequence * seq = selectedWidgets_[i]->sequence();
            ScopedSequenceChange lock(scene_, seq);
                seq->setStart(newstart);
        }

        autoScrollView_(e->pos());

        if (hoverWidget_)
            setCurrentTime_(hoverWidget_->sequence()->start());

        e->accept();
        return;
    }

    // ---- drag sequence(s) left edge ----

    if (action_ == A_DRAG_LEFT_)
    {
        for (int i=0; i<selectedWidgets_.size(); ++i)
        {
            Sequence * seq = selectedWidgets_[i]->sequence();
            Double newstart = std::max((Double)0, std::min(seq->end() - Sequence::minimumLength(),
                                    dragStartTimes_[i] + deltaTime));
            Double change = (dragStartTimes_[i] - newstart) * seq->speed();
            Double newlength = std::max(Sequence::minimumLength(), dragStartLengths_[i] + change );
            Double newOffset = dragStartOffsets_[i];
            if (e->modifiers() & modifierDragWithOffset_)
                newOffset -= change;
            ScopedSequenceChange lock(scene_, seq);
                seq->setStart(newstart);
                seq->setTimeOffset(newOffset);
                seq->setLength(newlength);
        }

        autoScrollView_(e->pos());

        if (hoverWidget_)
            setCurrentTime_(hoverWidget_->sequence()->start());

        e->accept();
        return;
    }

    // ---- drag sequence(s) right edge ----

    if (action_ == A_DRAG_RIGHT_)
    {
        for (int i=0; i<selectedWidgets_.size(); ++i)
        {
            Sequence * seq = selectedWidgets_[i]->sequence();
            Double newlength = std::max(Sequence::minimumLength(),
                                        dragStartLengths_[i] + deltaTime * seq->speed() );
            ScopedSequenceChange lock(scene_, seq);
            seq->setLength(newlength);
        }

        autoScrollView_(e->pos());

        if (hoverWidget_)
            setCurrentTime_(hoverWidget_->sequence()->end());

        e->accept();
        return;
    }

    // --- selection frame ---

    if (action_ == A_SELECT_FRAME_)
    {
        QPoint ds(viewToScreen(dragStartPosV_));
        QRect r(selectRect_);
        selectRect_.setLeft(std::min(ds.x(), e->pos().x()));
        selectRect_.setRight(std::max(ds.x(), e->pos().x()));
        selectRect_.setTop(std::min(ds.y(), e->pos().y()));
        selectRect_.setBottom(std::max(ds.y(), e->pos().y()));

        update(updateRect_(r, penSelectFrame_));
        update(updateRect_(selectRect_, penSelectFrame_));

        // check the widgets touched by selection frame
        for (auto w : framedWidgets_)
            if (!selectRect_.intersects(w->geometry()))
                update(updateRect_(w->geometry(), penFramedWidget_));
        framedWidgets_.clear();
        for (auto w : sequenceWidgets_)
        if (selectRect_.intersects(w->geometry()))
        {
            framedWidgets_.append(w);
            update(updateRect_(w->geometry(), penFramedWidget_));
        }

        autoScrollView_(e->pos());
    }

/*
    if (dragSequence_)
    {
        Double x = space_.mapXTo((Double)e->x() / width()),
              dx = x - dragStartTime_;

        // ---- drag sequence position ----

        Double newstart = std::max((Double)0, dragStartSeqTime_ + dx);
        dragSequence_->setStart(newstart);

        // deterimine new-track
        auto dragEndTrack = trackForY(e->y());
        if (dragEndTrack != dragEndTrack_ || dragEndTrack != dragStartTrack_)
        {
            dragEndTrack_ = dragEndTrack;
            update();
        }

        e->accept();
        return;
    }
*/
}

void TrackView::mouseReleaseEvent(QMouseEvent * e)
{
    bool multisel = (e->modifiers() & modifierMultiSelect_);

    if (hoverWidget_)
        hoverWidget_->setStatusTip(statusSeqNormal);

    if (action_ == A_SELECT_FRAME_)
    {
        update(updateRect_(selectRect_, penSelectFrame_));

        for (auto w : framedWidgets_)
            update(updateRect_(w->geometry(), penFramedWidget_));

        selectSequenceWidgets_(selectRect_, multisel? FLIP_ : SELECT_);

        action_ = A_NOTHING_;
        framedWidgets_.clear();

        e->accept();
        return;
    }

    action_ = A_NOTHING_;
}

void TrackView::autoScrollView_(const QPoint& p)
{
    // shift x viewspace
    int scrOffset = 0;
    if (p.x() > width())
        scrOffset = std::min(10, p.x() - width());
    else if (p.x() < 0)
        scrOffset = std::max(-10, p.x());

    if (scrOffset)
    {
        Double delta = space_.mapXDistanceTo((Double)scrOffset/width());
        space_.addX(delta * 1.2);
        updateWidgetsViewSpace_();
        emit viewSpaceChanged(space_);
    }

    // TODO shift y viewspace
}

void TrackView::selectSequenceWidget_(SequenceWidget * w, SelectState_ s)
{
    if (w->selected() && (s == UNSELECT_ || s == FLIP_))
    {
        w->setSelected(false);
        selectedWidgets_.removeOne(w);
    }
    else
    if (!w->selected() && (s == SELECT_ || s == FLIP_))
    {
        w->setSelected(true);
        selectedWidgets_.append(w);
    }
}

void TrackView::selectSequenceWidgets_(const QRect & r, SelectState_ s)
{
    if (r.size().isNull())
        return;

    for (auto w : sequenceWidgets_)
    {
        if (r.intersects(w->geometry()))
            selectSequenceWidget_(w, s);
    }
}

void TrackView::clearSelection_()
{
    for (auto w : selectedWidgets_)
        w->setSelected(false);

    selectedWidgets_.clear();
}

/*void TrackView::sequenceTimeChanged(Sequence * )
{
    if (SequenceWidget * s = widgetForSequence_(seq))
        updateWidgetViewSpace_(s);
}*/

void TrackView::onParameterChanged_(Parameter * p)
{
    if (auto seq = qobject_cast<Sequence*>(p->object()))
        onSequenceChanged_(seq);
}

void TrackView::onSequenceChanged_(Sequence * seq)
{
    // This slot signals either changes in time
    // or changes to the containing data
    if (SequenceWidget * s = widgetForSequence_(seq))
    {
        // if time didn't change
        if (!updateWidgetViewSpace_(s))
        // then content must have changed
        {
            s->updateValueRange();
            s->update();
        }

        // go through all influenced widgets
        // if 's' is a modulator
        for (auto w : s->influencedWidgets())
        if (!updateWidgetViewSpace_(w))
        {
            w->updateValueRange();
            w->update();
        }
    }
}

void TrackView::onObjectChanged_(Object * obj)
{
    if (auto seq = qobject_cast<Sequence*>(obj))
        if (auto w = widgetForSequence_(seq))
        {
            w->update();
            w->updateName();
            for (auto w2 : w->influencedWidgets())
                w2->update();
        }
}

Track * TrackView::trackForY_(int y) const
{
    y -= offsetY_;

    for (auto i = trackY_.begin(); i != trackY_.end(); ++i)
    {
        if (y >= i.value() && y <= i.value() + trackHeight(i.key()))
            return i.key();
    }

    return 0;
}

QRect TrackView::trackRect_(Track * t) const
{
    return QRect(0, trackY(t), width(), trackHeight(t));
}

QRect TrackView::updateRect_(const QRect &rect, const QPen &pen)
{
    const int w = pen.width() / 2;
    return rect.adjusted(-w,-w,w+1,w+1);
}

int TrackView::trackIndex_(Track * t)
{
    return tracks_.indexOf(t);
}




void TrackView::createEditActions_()
{
    // remove old actions
    for (auto a : editActions_)
        if (!actions().contains(a))
        {
//            if (a->menu())
//                a->menu()->deleteLater();
            a->deleteLater();
        }

    editActions_.clear();

    QAction * a;
    //QMenu * m;

    // ---------- actions for a single sequence ---------------

    if (hoverWidget_ && selectedWidgets_.size() < 2)
    {
        Sequence * seq = hoverWidget_->sequence();
        // local copy of hoverWidget_
        // ('cause it goes away in lambdas)
        SequenceWidget * widget = hoverWidget_;

        // title
        editActions_.addTitle(seq->name(), this);

        editActions_.addSeparator(this);

        // set color
        a = editActions_.addAction(tr("Change color"), this);
        a->setStatusTip(tr("Sets a custom color for the sequence"));
        QMenu * sub = ObjectMenu::createColorMenu(this);
        a->setMenu(sub);
        connect(sub, &QMenu::triggered, [this, seq, widget](QAction*a)
        {
            QColor col = a->data().value<QColor>();
            seq->setColor(col);
            widget->updateColors();
        });

        editActions_.addSeparator(this);

        // copy
        a = editActions_.addAction(tr("Copy"), this);
        a->setStatusTip(tr("Copies the selected sequence to the clipboard"));
        connect(a, &QAction::triggered, [this, seq]()
        {
            auto data = new ObjectTreeMimeData();
            data->storeObjectTree(seq);
            application()->clipboard()->setMimeData(data);
        });

        // cut
        a = editActions_.addAction(tr("Cut"), this);
        a->setStatusTip(tr("Moves the selected sequence to the clipboard"));
        a->setEnabled(seq->canBeDeleted());
        connect(a, &QAction::triggered, [this, seq]()
        {
            auto data = new ObjectTreeMimeData();
            data->storeObjectTree(seq);
            application()->clipboard()->setMimeData(data);
            if (deleteObject_(seq))
                emit sequenceSelected(0);
        });

        // delete
        a = editActions_.addAction(tr("Delete"), this);
        a->setStatusTip(tr("Deletes the selected sequence"));
        a->setEnabled(seq->canBeDeleted());
        connect(a, &QAction::triggered, [this, seq]()
        {
            if (deleteObject_(seq))
                emit sequenceSelected(0);
        });

    }
    else

    // ---------- multi-sequence actions -----------

    if (hoverWidget_ && selectedWidgets_.size() > 1)
    {
        // title
        editActions_.addTitle(tr("%1 sequences").arg(selectedWidgets_.size()), this);

        editActions_.addSeparator(this);

        // set color
        a = editActions_.addAction(tr("Change color"), this);
        a->setStatusTip(tr("Sets a custom color for the sequence"));
        QMenu * sub = ObjectMenu::createColorMenu(this);
        a->setMenu(sub);
        connect(sub, &QMenu::triggered, [this](QAction*a)
        {
            QColor col = a->data().value<QColor>();
            for (auto w : selectedWidgets_)
            {
                w->sequence()->setColor(col);
                w->updateColors();
            }
        });

        editActions_.addSeparator(this);

        // copy
        a = editActions_.addAction(tr("Copy"), this);
        a->setStatusTip(tr("Copies all selected sequences to the clipboard"));
        connect(a, &QAction::triggered, [this]()
        {
            QList<Object*> seqs;
            QList<int> order;
            for (auto w : selectedWidgets_)
            {
                seqs.append(w->sequence());
                order.append(trackIndex_(w->sequence()->parentTrack()));
            }

            auto data = new ObjectTreeMimeData();
            data->storeObjectTrees(seqs);
            data->storeOrder(order);
            application()->clipboard()->setMimeData(data);
        });

        // cut
        a = editActions_.addAction(tr("Cut"), this);
        a->setStatusTip(tr("Moves all selected sequences to the clipboard"));
        connect(a, &QAction::triggered, [this]()
        {
            QList<Object*> seqs;
            QList<int> order;
            for (auto w : selectedWidgets_)
            {
                seqs.append(w->sequence());
                order.append(trackIndex_(w->sequence()->parentTrack()));
            }

            auto data = new ObjectTreeMimeData();
            data->storeObjectTrees(seqs);
            data->storeOrder(order);
            application()->clipboard()->setMimeData(data);

            emit sequenceSelected(0);
            for (auto s : seqs)
                if (s->canBeDeleted())
                    deleteObject_(s);

        });

        // delete
        a = editActions_.addAction(tr("Delete"), this);
        a->setStatusTip(tr("Delete all selected sequences"));
        connect(a, &QAction::triggered, [this]()
        {
            QList<Object*> seqs;
            for (auto w : selectedWidgets_)
                seqs.append(w->sequence());

            emit sequenceSelected(0);
            for (auto s : seqs)
                if (s->canBeDeleted())
                    deleteObject_(s);
        });

    }
    else

    // -------------- actions for a track ------------------

    if (selTrack_)
    {
        a = editActions_.addAction(tr("New sequence"), this);
        a->setStatusTip(tr("Creates a new sequence at the current time"));
        connect(a, &QAction::triggered, [this]()
        {
            if (auto trackf = qobject_cast<TrackFloat*>(selTrack_))
            {
                Sequence * seq = ObjectFactory::createSequenceFloat(trackf->name());
                seq->setStart(currentTime_);
                editor_->addObject(trackf, seq);
                nextFocusSequence_ = seq;

                updateTrack(selTrack_);
                assignModulatingWidgets_();
            }
        });

        editActions_.addSeparator(this);

        // paste
        if (application()->clipboard()->mimeData()->formats().contains(
                    ObjectTreeMimeData::objectMimeType))
        {
            const ObjectTreeMimeData * data
                    = qobject_cast<const ObjectTreeMimeData*>(application()->clipboard()->mimeData());
            if (data)
            {
                // paste single
                if (data->getNumObjects() == 1)
                {
                    a = editActions_.addAction(tr("Paste on %1").arg(selTrack_->name()), this);
                    a->setStatusTip(tr("Pastes the sequence from the clipboard to the selected track"));
                    connect(a, &QAction::triggered, [this](){ paste_(); });
                }
                // paste all
                else
                {
                    a = editActions_.addAction(tr("Paste all"), this);
                    a->setStatusTip(tr("Pastes all sequences from the clipboard, starting at the selected track"));
                    connect(a, &QAction::triggered, [this](){ paste_(); });

                    a = editActions_.addAction(tr("Paste all on %1").arg(selTrack_->name()), this);
                    a->setStatusTip(tr("Pastes all sequences from the clipboard after each other on the selected track"));
                    connect(a, &QAction::triggered, [this](){ paste_(true); });
                }
            }
        }
    }
}

bool TrackView::deleteObject_(Object * o)
{
    return editor_->deleteObject(o);
}

bool TrackView::paste_(bool single_track)
{
    if (!selTrack_)
        return false;

    const ObjectTreeMimeData * data = qobject_cast<const ObjectTreeMimeData*>
            (application()->clipboard()->mimeData());
    if (!data)
        return false;

    clearSelection_();

    QString error;

    // single sequence
    if (data->getNumObjects() == 1)
    {
        Object * o = data->getObjectTree();
        if (Sequence * s = qobject_cast<Sequence*>(o))
        {
            if (selTrack_->isSaveToAdd(s, error))
            {
                s->setStart(currentTime_);
                nextFocusSequence_ = s;

                if (editor_->addObject(selTrack_, s))
                    return true;

            }
            else
                QMessageBox::warning(this, tr("Can not paste"), error);
        }
        nextFocusSequence_ = 0;
        delete o;
        return false;
    }
    // multi sequences
    else
    {
        QString errors;

        QList<Object*> objs = data->getObjectTrees();

        // all on same track
        if (single_track)
        {
            Double time = currentTime_;
            for (int i=0; i<objs.size(); ++i)
            if (Sequence * s = qobject_cast<Sequence*>(objs[i]))
            {
                if (!selTrack_->isSaveToAdd(s, error))
                {
                    errors += error + "\n";
                }
                else
                {
                    s->setStart(time);

                    if (editor_->addObject(selTrack_, s))
                    {
                        time = s->end();
                        continue;
                    }

                }
                delete s;
                objs[i] = 0;
            }

            // select
            for (auto o : objs)
                if (Sequence * s = qobject_cast<Sequence*>(o))
                    if (auto w = widgetForSequence_(s))
                        selectSequenceWidget_(w, SELECT_);

            if (!errors.isEmpty())
                QMessageBox::warning(this, tr("Can not paste"), errors);

            return true;
        }
        else
        {
            // get order
            QList<int> order;
            if (data->hasOrder())
                order = data->getOrder();
            else
            {
                // make some up
                for (auto i=0; i<objs.size(); ++i)
                    order.append(i);
            }

            if (order.size() == objs.size())
            {
                // get first track in order
                int mint = order[0];
                for (auto i : order)
                    mint = std::min(mint, i);

                int selt = trackIndex_(selTrack_);

                // get left-most start time
                Double start = -1;
                for (auto o : objs)
                    if (Sequence * s = qobject_cast<Sequence*>(o))
                        if (start < 0 || s->start() < start)
                            start = s->start();

                if (start >= 0)
                {
                    for (int i = 0; i<objs.size(); ++i)
                    if (Sequence * s = qobject_cast<Sequence*>(objs[i]))
                    {
                        s->setStart(currentTime_ + s->start() - start);
                        // find track
                        int tracknum = selt + order[i] - mint;
                        //MO_DEBUG("pasting '" << s->idName() << "' at "
                        //         << tracknum << ":" << s->start());
                        // out of tracks?
                        if (tracknum < tracks_.size())
                        {
                            if (!tracks_[tracknum]->isSaveToAdd(s, error))
                            {
                                errors += error + "\n";
                            }

                            else
                            // add
                            if (editor_->addObject(tracks_[tracknum], s))
                                continue;
                        }
                            else MO_WARNING("skipping sequence '" << s->name() << "' "
                                            "because there is no track left");
                        delete s;
                        objs[i] = 0;
                    }
                    // select
                    for (auto o : objs)
                        if (Sequence * s = qobject_cast<Sequence*>(o))
                            if (auto w = widgetForSequence_(s))
                                selectSequenceWidget_(w, SELECT_);

                    if (!errors.isEmpty())
                        QMessageBox::warning(this, tr("Can not paste"), errors);

                    return true;
                }
                else
                    MO_WARNING("no sequences found in clipboard with '"
                               << objs.size() << " objects. ");
            }
            else
                MO_WARNING("order size (" << order.size() << ") does not match "
                           "number of sequences (" << objs.size() << ")");

            for (auto o : objs)
                delete o;
        }
    }

    return false;
}

} // namespace GUI
} // namespace MO
