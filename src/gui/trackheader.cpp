/** @file trackheader.cpp

    @brief Track/Names area working with TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/
#include "io/memory.h"

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

#include "trackheader.h"
#include "trackview.h"
#include "io/error.h"
#include "widget/trackheaderwidget.h"

namespace MO {
namespace GUI {

TrackHeader::TrackHeader(TrackView * trackView, QWidget *parent) :
    QWidget     (parent),
    trackView_  (trackView),
    offsetY_    (0)
{
    MO_ASSERT(trackView_, "TrackView not set for TrackHeader");

    setMinimumSize(100, trackView_->minimumHeight());
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    setAutoFillBackground(true);

    setPalette(trackView_->palette());
}

void TrackHeader::setVerticalOffset(int y)
{
    bool changed = (offsetY_ != y);
    offsetY_ = y;
    if (changed)
    {
        updateWidgetsViewSpace_();
        update();
    }
}


void TrackHeader::clearTracks()
{
    tracks_.clear();
    for (auto w : widgets_)
        w->deleteLater();
    widgets_.clear();
}

void TrackHeader::setTracks(const QList<Track *> &tracks)
{
    clearTracks();

    tracks_ = tracks;

    TrackHeaderWidget * w;
    for (auto t : tracks)
    {
         widgets_.append( w = new TrackHeaderWidget(t, this) );
         w->setVisible(true);
         connect(w, &TrackHeaderWidget::heightChange, [=](int h)
         {
            trackView_->setTrackHeight(t, h);
            updateWidgetsViewSpace_();
            update();
         });
    }

    updateWidgetsViewSpace_();
    update();
}


void TrackHeader::updateWidgetsViewSpace_()
{
    for (auto w : widgets_)
    {
        const int h = trackView_->trackHeight(w->track()),
                  y = trackView_->trackY(w->track());

        w->setFixedHeight(h);
        w->move(0, y);
    }
}

void TrackHeader::paintEvent(QPaintEvent * )
{
    QPainter p(this);

    p.setPen(QColor(80,80,80));
    for (auto t : tracks_)
    {
        const int y = trackView_->trackY(t)
                        + trackView_->trackHeight(t)
                        + trackView_->trackSpacing() / 2;
        p.drawLine(0, y, width(), y);
    }

}

} // namespace GUI
} // namespace MO
