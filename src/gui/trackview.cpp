/** @file trackview.cpp

    @brief Track view / Sequencer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

#include "trackview.h"

namespace MO {
namespace GUI {

TrackView::TrackView(QWidget *parent) :
    QGraphicsView(parent)
    //QWidget(parent)
{
    setMinimumSize(320,240);
    /*
    QPalette p(palette());
    p.setColor(QPalette::Background, QColor(50,50,50));
    setPalette(p);
    setAutoFillBackground(true);
    */
    scene_ = new QGraphicsScene(this);
    scene_->setBackgroundBrush(QBrush(QColor(150,150,150)));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setScene(scene_);

    createItems_();

}




void TrackView::createItems_()
{
    auto r = scene_->addRect(10,10,50000,20);
    auto t = scene_->addText("blabla");
    t->setParentItem(r);
/*
    QPalette p(palette());
    p.setColor(QPalette::Background, QColor(80,120,80));

    auto w = new QWidget(this);
    w->setGeometry(10,10,5000,19);
    w->setAutoFillBackground(true);
    w->setPalette(p);

    w = new QWidget(this);
    w->setGeometry(10,30,5000,19);
    w->setAutoFillBackground(true);
    w->setPalette(p);

    w = new QWidget(this);
    w->setGeometry(10,50,100,19);
    w->setAutoFillBackground(true);
    w->setPalette(p);
*/
}


void TrackView::setViewSpace(const UTIL::ViewSpace & s)
{
    space_ = s;


}

} // namespace GUI
} // namespace MO
