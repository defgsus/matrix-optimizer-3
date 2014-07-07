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
        p.setPen(QColor(255,255,255));
        p.setBrush(Qt::NoBrush);
        p.drawRect(trackView_->selectRect_);
    }

}

} // namespace GUI
} // namespace MO
