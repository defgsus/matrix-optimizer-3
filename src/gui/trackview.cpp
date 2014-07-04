/** @file trackview.cpp

    @brief Track view / Sequencer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#include <QDebug>
/*
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
*/
#include <QPalette>

#include "trackview.h"
#include "trackheader.h"
#include "widget/sequencewidget.h"
#include "object/sequencefloat.h"
#include "object/track.h"
#include "io/error.h"


namespace MO {
namespace GUI {

TrackView::TrackView(QWidget *parent) :
    QWidget         (parent),
    scene_          (0),
    header_         (0),

    defaultTrackHeight_     (30),
    trackYSpacing_          (2)
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

    updateViewSpace_();
}

void TrackView::updateViewSpace_()
{
    const int trackHeight_ = 30;
    int k=0;
    for (auto s : sequenceWidgets_)
    {
        QRect r(0, k * trackHeight_, 10, trackHeight_ - 1);
        r.setLeft(space_.mapXFrom(s->sequence()->start()) * width());
        r.setRight(space_.mapXFrom(s->sequence()->end()) * width());
        s->setGeometry(r);
        ++k;
    }
}

void TrackView::setScene(Scene * scene)
{
    scene_ = scene;

    if (scene_ == 0)
    {
        clearTracks();
        return;
    }
}

void TrackView::clearTracks()
{
    tracks_.clear();
    trackY_.clear();

    for (auto s : sequenceWidgets_)
        s->deleteLater();

    sequenceWidgets_.clear();
}


void TrackView::setTracks(const QList<Track *> &tracks, bool send_signal)
{
    // removed previous content
    clearTracks();
    if (tracks.empty())
        return;

    // determine scene
    if (!scene_)
    {
        setScene(tracks[0]->sceneObject());
    }

    MO_ASSERT(scene_, "Scene not set in TrackView::setTracks()");

    tracks_ = tracks;

    if (send_signal)
        emit tracksChanged();

    calcTrackY_();

    createSequenceWidgets_();
}

void TrackView::calcTrackY_()
{
    // set track positions
    int y = 0;
    for (auto t : tracks_)
    {
        const int h = trackHeight(t);
        trackY_.insert(t, y);
        y += h + trackYSpacing_;
    }
}


int TrackView::trackHeight(Track * t) const
{
    return trackHeights_.value(t->idName(), defaultTrackHeight_);
}

int TrackView::trackY(Track * t) const
{
    return trackY_.value(t, 0);
}

void TrackView::createSequenceWidgets_()
{
    SequenceFloat * s;
    for (int i=0; i<30; ++i)
    {
        sequenceWidgets_.append(
                    new SequenceWidget(s = new SequenceFloat(this), this)
                    );
        s->setStart((Double)rand()/RAND_MAX * 60);
        s->setLength((Double)rand()/RAND_MAX * 60);
    }
}






} // namespace GUI
} // namespace MO
