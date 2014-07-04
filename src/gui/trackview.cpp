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
#include "widget/sequencewidget.h"
#include "object/sequencefloat.h"

namespace MO {
namespace GUI {

TrackView::TrackView(QWidget *parent) :
    QWidget(parent)
{
    setMinimumSize(320,240);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPalette p(palette());
    p.setColor(QPalette::Background, QColor(50,50,50));
    setPalette(p);
    setAutoFillBackground(true);

    createItems_();
}


void TrackView::createItems_()
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



} // namespace GUI
} // namespace MO
