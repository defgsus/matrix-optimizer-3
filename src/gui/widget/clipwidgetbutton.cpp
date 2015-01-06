/** @file clipwidgetbutton.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 14.10.2014</p>
*/

#include <QPainter>
#include <QMouseEvent>
#include <QImage>

#include "clipwidgetbutton.h"

namespace MO {
namespace GUI {

ClipWidgetButton::ClipWidgetButton(Type type, QWidget * parent)
    : QWidget       (parent),
      type_         (type),
      state_        (S_OFF),
      hasFocus_     (false)
{
    setMouseTracking(true);

    setFixedSize(13, 17);

    updateColors_();
}

void ClipWidgetButton::setState(State state)
{
    state_ = state;
    update();
}

void ClipWidgetButton::resizeEvent(QResizeEvent *)
{
    if (type_ == T_PLAY)
    {
        poly_.clear();
        poly_ << QPoint(0, height() - 2)
              << QPoint(width() - 1, height() / 2)
              << QPoint(0, 2);
    }
}

void ClipWidgetButton::updateColors_()
{
    pen_ = QPen(QColor(0,0,0));
    penF_ = QPen(QColor(200,230,200));

    const int alpha = 100;
    brushStopped_ = QBrush(QColor(0,0,0,alpha));
    brushTriggered_ = QBrush(QColor(150,150,200,alpha));
    brushPlaying_ = QBrush(QColor(200,250,200,alpha));
}

void ClipWidgetButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    p.setPen(hasFocus_ ? penF_ : pen_);

    if (state_ == S_OFF)
        p.setBrush(brushStopped_);
    else if (state_ == S_TRIGGERED)
        p.setBrush(brushTriggered_);
    else
        p.setBrush(brushPlaying_);



    if (type_ == T_STOP)
    // rect
        p.drawRect(0,3,width()-1, height()-6);
    // triangle
    else
    {
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawPolygon(poly_);
    }
}


void ClipWidgetButton::enterEvent(QEvent *)
{
    hasFocus_ = true;
    update();
}

void ClipWidgetButton::leaveEvent(QEvent *)
{
    hasFocus_ = false;
    update();
}


void ClipWidgetButton::mousePressEvent(QMouseEvent * )
{
    emit clicked();
}


} // namespace GUI
} // namespace MO
