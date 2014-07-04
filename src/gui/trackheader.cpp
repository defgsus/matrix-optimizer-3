/** @file trackheader.cpp

    @brief Track/Names area working with TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#include "trackheader.h"
#include "trackview.h"
#include "io/error.h"
#include "widget/trackheaderwidget.h"

namespace MO {
namespace GUI {

TrackHeader::TrackHeader(TrackView * trackView, QWidget *parent) :
    QWidget     (parent),
    trackView_  (trackView)
{
    MO_ASSERT(trackView_, "TrackView not set for TrackHeader");

    setMinimumSize(100, trackView_->minimumHeight());
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    setAutoFillBackground(true);

    setPalette(trackView_->palette());
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
    }

    updateWidgets_();
}


void TrackHeader::updateWidgets_()
{
    for (auto w : widgets_)
    {
        const int h = trackView_->trackHeight(w->track()),
                  y = trackView_->trackY(w->track());

        w->setFixedHeight(h);
        w->move(0, y);
    }
}

} // namespace GUI
} // namespace MO
