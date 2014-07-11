/** @file trackviewoverpaint.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/7/2014</p>
*/
#include "io/memory.h"

#include <QPainter>
#include <QPaintEvent>

#include "trackviewoverpaint.h"
#include "trackview.h"
#include "widget/sequencewidget.h"

namespace MO {
namespace GUI {

TrackViewOverpaint::TrackViewOverpaint(TrackView * trackView, QWidget *parent) :
    QWidget     (parent),
    trackView_  (trackView)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void TrackViewOverpaint::paintEvent(QPaintEvent * )
{
    QPainter p(this);
    p.setBrush(Qt::NoBrush);

    // current time
    p.setPen(trackView_->penCurrentTime_);
    int x = trackView_->space_.mapXFrom(trackView_->currentTime_) * width();
    p.drawLine(x, 0, x, height());

    // frame selection
    if (trackView_->action_ == TrackView::A_SELECT_FRAME_)
    {
        // framed widgets
        p.setPen(trackView_->penFramedWidget_);
        for (auto w : trackView_->framedWidgets_)
            p.drawRect(w->geometry());

        // selection frame
        p.setPen(trackView_->penSelectFrame_);
        p.drawRect(trackView_->selectRect_);
    }
}

} // namespace GUI
} // namespace MO
