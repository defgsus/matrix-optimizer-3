/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/23/2015</p>
*/

#include <QTimer>
#include <QPaintEvent>
#include <QPainter>

#include "blinklamp.h"


namespace MO {
namespace GUI {


BlinkLamp::BlinkLamp(QWidget *parent)
    : QWidget       (parent)
    , timer_        (new QTimer(this))
    , doBlink_      (0)
{
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, [=]()
    {
        doBlink_ = 0;
        update();
    });
}

BlinkLamp::BlinkLamp(const QSize &fixedSize, QWidget *parent)
    : BlinkLamp     (parent)
{
    setFixedSize(fixedSize);
}

void BlinkLamp::blink(int duration_ms)
{
    doBlink_ = duration_ms;
    update();
}

void BlinkLamp::paintEvent(QPaintEvent * )
{
    QPainter p(this);

    p.setPen(Qt::NoPen);
    if (doBlink_)
        p.setBrush(Qt::green);
    else
        p.setBrush(Qt::darkGreen);

    p.drawRect(rect());

    if (doBlink_)
        timer_->start(doBlink_);
}


} // namespace GUI
} // namespace MO
