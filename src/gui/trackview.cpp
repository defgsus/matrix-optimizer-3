/** @file trackview.cpp

    @brief Track view / Sequencer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/3/2014</p>
*/

#include <QDebug>

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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setAlignment(0);

    /*
    QPalette p(palette());
    p.setColor(QPalette::Background, QColor(50,50,50));
    setPalette(p);
    setAutoFillBackground(true);
    */
    scene_ = new QGraphicsScene(this);
    scene_->setBackgroundBrush(QBrush(QColor(150,150,150)));

    setScene(scene_);

    createItems_();

}


QGraphicsRectItem * seq;

void TrackView::createItems_()
{
    /*
    auto r = scene_->addRect(10,10,50000,20,QPen(), QBrush(QColor(100,120,100)));
    auto t = scene_->addText("blabla");
    t->setParentItem(r);
    */
    seq = scene_->addRect(50,40,50,20, QPen(), QBrush(QColor(100,120,100)));

    scene_->addLine(0,0, 0,300);
    scene_->addLine(100,0, 100,300);
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

    QRectF r(seq->rect());
    r.setLeft(space_.mapXFrom(0) * width());
    r.setRight(space_.mapXFrom(1) * width());
    seq->setRect(r);

    //QTransform t;
    //t.scale(1.0 / space_.scaleX(), 1);
    //t.translate(-space_.x() * width(), 0);
    //setTransform(t);
    /*
    QRectF r = sceneRect();
    qDebug() << r;
    r.moveLeft(r.left() + space_.x());//space_.x());
    qDebug() << r;
    setSceneRect(r);
    */
    //setSceneRect(QRectF(space_.x() * width(), sceneRect().top(),
    //            width() / space_.scaleX(), sceneRect().height()));
    //qDebug() << sceneRect();
}

} // namespace GUI
} // namespace MO
