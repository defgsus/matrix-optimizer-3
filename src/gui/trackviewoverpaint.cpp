/** @file trackviewoverpaint.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/7/2014</p>
*/

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

    if (trackView_->action_ == TrackView::A_SELECT_FRAME_)
    {
        p.setBrush(Qt::NoBrush);

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
