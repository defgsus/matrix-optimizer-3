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
#include "trackheader.h"
#include "widget/sequencewidget.h"
#include "object/sequencefloat.h"
#include "object/track.h"
#include "object/scene.h"
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

    selTrack_               (0),
    nextFocusSequence_      (0),
    currentTime_            (0),
    dragSequence_           (0),

    defaultTrackHeight_     (30),
    trackSpacing_           (2)
{
    setMinimumSize(320,240);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(50,50,50));
    setPalette(p);
    setAutoFillBackground(true);

    header_ = new TrackHeader(this, this);
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

void TrackView::paintEvent(QPaintEvent * e)
{
    QPainter p(this);

    // background
#if (1)
    p.setPen(QColor(70,70,70));
    for (auto t : tracks_)
    {
        const int y = trackY(t) + trackHeight(t) + trackSpacing_ / 2
                    - offsetY_;
        p.drawLine(0, y, width(), y);
    }
#else
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(QColor(50,50,50).darker(120)));
    int k=1;
    for (auto t : tracks_)
    {
        if (k & 1)
            p.drawRect(0, trackY(t), width(), trackHeight(t));
        ++k;
    }
#endif

    QWidget::paintEvent(e);
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

void TrackView::updateWidgetViewSpace_(SequenceWidget * s)
{
    const int h = trackHeight(s->track()),
              y = trackY(s->track()) - offsetY_;
    QRect r(0, y, 10, h);

    r.setLeft(space_.mapXFrom(s->sequence()->start()) * width());
    r.setRight(space_.mapXFrom(s->sequence()->end()) * width());
    s->setGeometry(r);
}

void TrackView::clearTracks()
{
    tracks_.clear();
    trackY_.clear();

    for (auto s : sequenceWidgets_)
        s->deleteLater();

    sequenceWidgets_.clear();

    header_->clearTracks();
}


void TrackView::setTracks(const QList<Track *> &tracks, bool send_signal)
{
    // removed previous content
    clearTracks();
    if (tracks.empty())
        return;

    // determine scene
    scene_ = tracks[0]->sceneObject();

    MO_ASSERT(scene_, "Scene not set in TrackView::setTracks()");

    tracks_ = tracks;

    calcTrackY_();

    for (auto t : tracks_)
        createSequenceWidgets_(t);

    updateWidgetsViewSpace_();

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
    return trackY_.value(t, 0);
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

    for (Sequence * seq : t->sequences())
    {
        // create the widget
        auto w = new SequenceWidget(t, seq, this);
        sequenceWidgets_.append( w );

        // initialize
        w->setVisible(true);
        connect(seq, SIGNAL(timeChanged(MO::Sequence*)),
                this, SLOT(sequenceTimeChanged(MO::Sequence*)));

        // set the focus
        if (seq == nextFocusSequence_)
        {
            w->setFocus();
            nextFocusSequence_ = 0;
        }
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


void TrackView::mousePressEvent(QMouseEvent * e)
{
    /*QWidget::mousePressEvent(e);

    if (e->isAccepted())
        return;
    */

    // --- clicked on sequence ---

    if (SequenceWidget * seqw = qobject_cast<SequenceWidget*>(childAt(e->pos())))
    {
        if (e->button() == Qt::LeftButton)
        {
            dragSequence_ = seqw->sequence();
            dragStartPos_ = e->pos();
            dragStartTime_ = space_.mapXTo((Double)e->x()/width());
            dragStartSeqTime_ = dragSequence_->start();

            e->accept();
            return;
        }
    }


    // --- clicked on track ---

    selTrack_ = trackForY(e->y());
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
}

void TrackView::mouseMoveEvent(QMouseEvent * e)
{
    if (dragSequence_)
    {
        Double x = space_.mapXTo((Double)e->x() / width()),
              dx = x - dragStartTime_;

        // ---- drag sequence position ----

        Double newstart = std::max((Double)0, dragStartSeqTime_ + dx);
        dragSequence_->setStart(newstart);

        // shift viewspace
        int scrOffset = 0;
        if (e->x() > width())
            scrOffset = std::min(10, e->x() - width());
        else if (e->x() < 0)
            scrOffset = std::max(-10, e->x());

        if (scrOffset)
        {
            Double delta = space_.mapXDistanceTo((Double)scrOffset/width());
            space_.addX(delta * 1.2);
            updateWidgetsViewSpace_();
            emit viewSpaceChanged(space_);
        }

        e->accept();
        return;
    }
}

void TrackView::mouseReleaseEvent(QMouseEvent *)
{
    dragSequence_ = 0;
}

void TrackView::sequenceTimeChanged(Sequence * seq)
{
    if (SequenceWidget * s = widgetForSequence_(seq))
        updateWidgetViewSpace_(s);
}

Track * TrackView::trackForY(int y) const
{
    for (auto i = trackY_.begin(); i != trackY_.end(); ++i)
    {
        if (y >= i.value() && y <= i.value() + trackHeight(i.key()))
            return i.key();
    }

    return 0;
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
            nextFocusSequence_ =
                scene_->createFloatSequence(selTrack_, currentTime_);
            updateTrack(selTrack_);
        });
    }
}


} // namespace GUI
} // namespace MO
