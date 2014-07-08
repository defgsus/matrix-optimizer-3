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


#include "trackview.h"
#include "trackviewoverpaint.h"
#include "trackheader.h"
#include "widget/sequencewidget.h"
#include "object/sequencefloat.h"
#include "object/trackfloat.h"
#include "object/scene.h"
#include "model/objecttreemodel.h"
#include "io/error.h"
#include "io/log.h"


namespace MO {
namespace GUI {

TrackView::TrackView(QWidget *parent) :
    QWidget         (parent),
    scene_          (0),
    header_         (0),

    offsetY_                (0),
    maxHeight_              (0),

    action_                 (A_NOTHING_),
    selTrack_               (0),
    nextFocusSequence_      (0),
    currentTime_            (0),
    hoverWidget_            (0),

    defaultTrackHeight_     (30),
    trackSpacing_           (2),
    modifierMultiSelect_    (Qt::CTRL)
{
    setMinimumSize(320,240);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);

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

    /*
    // when moving sequences
    if (dragSequence_ && dragEndTrack_ && dragStartTrack_ != dragEndTrack_)
    {
        int y = trackY(dragEndTrack_) - 2,
            h = trackHeight(dragEndTrack_) + 4,
            x = space_.mapXFrom(dragSequence_->start()) * width(),
            w = space_.mapXDistanceFrom(dragSequence_->length()) * width();

        p.setBrush(QBrush(QColor(255,255,255,30)));
        p.setPen(QPen(QColor(255,255,255,150)));
        p.drawRect(x, y, w, h);
    }
    */
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
        return true;
    }
    return false;
}

void TrackView::clearTracks()
{
    tracks_.clear();
    trackY_.clear();

    for (auto s : sequenceWidgets_)
        s->deleteLater();

    sequenceWidgets_.clear();
    selectedWidgets_.clear();

    offsetY_ = 0;

    header_->clearTracks();
}


void TrackView::setTracks(const QList<Track *> &tracks, bool send_signal)
{
    // removed previous content
    clearTracks();
    if (tracks.empty())
        return;

    // determine scene
    Scene * scene = tracks[0]->sceneObject();
    if (scene != scene_)
    {
        connect(scene, SIGNAL(objectChanged(MO::Object*)),
                this, SLOT(objectChanged(MO::Object*)));
        connect(scene, SIGNAL(sequenceChanged(MO::Sequence*)),
                this, SLOT(sequenceChanged(MO::Sequence*)));
    }
    scene_ = scene;

    MO_ASSERT(scene_, "Scene not set in TrackView::setTracks()");

    tracks_ = tracks;

    calcTrackY_();

    for (auto t : tracks_)
        createSequenceWidgets_(t);

    updateWidgetsViewSpace_();
    update();

    if (send_signal)
        emit tracksChanged();

    header_->setTracks(tracks);
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
    return trackHeights_.value(t->idName(), defaultTrackHeight_);
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
        // create the widget
        auto w = new SequenceWidget(t, seq, this);
        sequenceWidgets_.insert( w );

        // initialize
        w->setVisible(true);
        connect(w, SIGNAL(hovered(SequenceWidget*,bool)),
                this, SLOT(widgetHovered_(SequenceWidget*,bool)));
        //connect(seq, SIGNAL(timeChanged(MO::Sequence*)),
        //        this, SLOT(sequenceTimeChanged(MO::Sequence*)));

        // set the focus
        if (seq == nextFocusSequence_)
        {
            w->setFocus();
            nextFocusSequence_ = 0;
            emit sequenceSelected(seq);
        }
    }

    // needs to be on top
    overpaint_->raise();
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

void TrackView::widgetHovered_(SequenceWidget * w, bool on)
{
    if (action_ == A_NOTHING_)
        hoverWidget_ = on? w : 0;
}

void TrackView::mouseDoubleClickEvent(QMouseEvent * e)
{
    // doubleclick on sequence
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

    if (hoverWidget_)
    {
        // leftclick on sequence
        if (e->button() == Qt::LeftButton)
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

            // --- start drag pos ---

            if (isSelected_())
            {
                action_ = A_DRAG_POS_;
                //dragStartPos_ = e->pos();
                dragStartTime_ = space_.mapXTo((Double)e->x()/width());
                dragStartTrack_ = hoverWidget_->track();
                dragStartTimes_.clear();
                for (auto w : selectedWidgets_)
                    dragStartTimes_.append(w->sequence()->start());
            }

            e->accept();
            return;
        }
    }

    // --- clicked on track ---

    if (!multisel)
        clearSelection_();

    selTrack_ = trackForY_(e->y());
    currentTime_ = space_.mapXTo((Double)e->x() / width());

    createEditActions_();

    if (selTrack_)
    {
        // right-click on track
        if (e->button() == Qt::RightButton)
        {
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
        selectRect_ = QRect(dragStartPos_, QSize(1,1));
    }
}

void TrackView::mouseMoveEvent(QMouseEvent * e)
{
    if (action_ == A_NOTHING_)
        return;

    Double timeX = space_.mapXTo((Double)e->x() / width()),
           deltaTime = timeX - dragStartTime_;

    // ---- drag sequence position ----

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

    if (action_ == A_SELECT_FRAME_)
    {
        action_ = A_NOTHING_;
        update(updateRect_(selectRect_, penSelectFrame_));

        for (auto w : framedWidgets_)
            update(updateRect_(w->geometry(), penFramedWidget_));

        selectSequenceWidgets_(selectRect_, multisel? FLIP_ : SELECT_);

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

void TrackView::sequenceChanged(Sequence * seq)
{
    // This slot signals either changes in time
    // or changes to the containing data
    if (SequenceWidget * s = widgetForSequence_(seq))
    {
        if (!updateWidgetViewSpace_(s))
            s->update();
    }
}

void TrackView::objectChanged(Object * obj)
{
    if (auto seq = qobject_cast<Sequence*>(obj))
        if (auto w = widgetForSequence_(seq))
            w->update();
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

void TrackView::createEditActions_()
{
    // remove old actions
    for (auto a : editActions_)
        if (!actions().contains(a))
        {
            if (a->menu())
                a->menu()->deleteLater();
            a->deleteLater();
        }

    editActions_.clear();

    QAction * a;
    //QMenu * m;

    // actions for a track
    if (selTrack_)
    {
        editActions_.append( a = new QAction(tr("New sequence"), this) );
        connect(a, &QAction::triggered, [this]()
        {
            if (auto trackf = qobject_cast<TrackFloat*>(selTrack_))
            {
                nextFocusSequence_ =
                    scene_->model()->createFloatSequence(trackf, currentTime_);
                updateTrack(selTrack_);
            }
        });
    }
}


} // namespace GUI
} // namespace MO
