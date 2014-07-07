/** @file trackheaderwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#include <QDebug>
#include <QLayout>
#include <QLabel>

#include "trackheaderwidget.h"
#include "io/error.h"
#include "object/track.h"

namespace MO {
namespace GUI {


TrackHeaderWidget::TrackHeaderWidget(Track *track, QWidget *parent) :
    QWidget (parent),
    track_  (track)
{
    MO_ASSERT(track, "No Track given for TrackHeaderWidget");

    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(60,160,60));
    p.setColor(QPalette::Foreground, Qt::white);
    setPalette(p);

    setToolTip(track->namePath());

    layout_ = new QHBoxLayout(this);

    auto label = new QLabel(track->name().isEmpty()? "*unnamed*" : track->name(), this);
    layout_->addWidget(label);
}


} // namespace GUI
} // namespace MO
