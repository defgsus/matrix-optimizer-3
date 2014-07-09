/** @file trackheaderwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QMouseEvent>


#include "trackheaderwidget.h"
#include "io/error.h"
#include "object/track.h"

namespace MO {
namespace GUI {


TrackHeaderWidget::TrackHeaderWidget(Track *track, QWidget *parent) :
    QWidget     (parent),
    track_      (track),
    dragging_   (false)
{
    MO_ASSERT(track, "No Track given for TrackHeaderWidget");

    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(60,60,60));
    p.setColor(QPalette::Foreground, Qt::white);
    setPalette(p);

    setMouseTracking(true);

    setToolTip(track->namePath());

    layout_ = new QHBoxLayout(this);
    layout_->setMargin(2);

    auto label = new QLabel(track->name().isEmpty()? "*unnamed*" : track->name(), this);
    layout_->addWidget(label);
}


void TrackHeaderWidget::mousePressEvent(QMouseEvent * e)
{
    if (onEdge_ && e->button() == Qt::LeftButton)
    {
        dragging_ = true;
        dragStart_ = e->pos();
        dragStartHeight_ = height();

        e->accept();
    }
}

void TrackHeaderWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (!dragging_)
    {
        onEdge_ = e->y() >= height() - 2;
        setCursor(onEdge_? Qt::SplitVCursor : Qt::ArrowCursor);
    }
    else
    {
        int newHeight = dragStartHeight_ + e->y() - dragStart_.y();
        emit heightChange(std::max(10, newHeight));
        e->accept();
    }
}

void TrackHeaderWidget::mouseReleaseEvent(QMouseEvent * e)
{
    dragging_ = false;
    mouseMoveEvent(e);
}


} // namespace GUI
} // namespace MO
