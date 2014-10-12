/** @file clipwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include <QGraphicsRectItem>
#include <QPainter>

#include "clipwidget.h"
#include "gui/clipview.h"
#include "io/error.h"

namespace MO {
namespace GUI {

ClipWidget::ClipWidget(int x, int y, ClipView * parent)
    : QWidget       (parent),
      view_         (parent),
      x_            (x),
      y_            (y),
      hasFocus_     (false),
      isSelected_   (false)
{
    MO_ASSERT(!(x_==0 && y_==0), "illegal position for ClipWidget (0,0)");

    if (x_ == 0)
        type_ = T_ROW;
    else if (y_ == 0)
        type_ = T_COLUMN;
    else
        type_ = T_CLIP;

    switch (type_)
    {
        case T_COLUMN:
            setFixedSize(60, 20);
            penOutline_ = Qt::NoPen;
            brushBody_ = QBrush(QColor(100,100,100));
        break;

        case T_ROW:
            setFixedSize(20, 20);
            penOutline_ = Qt::NoPen;
            brushBody_ = QBrush(QColor(100,100,100));
        break;

        case T_CLIP:
            setFixedSize(60, 20);
            penOutline_ = QPen(QColor(40,40,40));
            brushBody_ = QBrush(QColor(60,70,60));
            if (rand()%20==0)
                brushBody_ = QBrush(QColor(130,170,130));
        break;
    }

    setMouseTracking(true);

    updateColors_();
}

void ClipWidget::updateColors_()
{
    brushBodyH_ = brushBodyS_ = brushBodySH_ = brushBody_;
    brushBodyH_.setColor(brushBody_.color().lighter(120));
    brushBodyS_.setColor(brushBody_.color().lighter(150));
    brushBodySH_.setColor(brushBodyS_.color().lighter(120));

    penOutlineS_ = penOutline_;
    penOutlineS_.setColor(penOutline_.color().lighter(150));
}

void ClipWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    if (isSelected_)
    {
        p.setPen(penOutlineS_);
        p.setBrush(hasFocus_? brushBodySH_ : brushBodyS_);
    }
    else
    {
        p.setPen(penOutline_);
        p.setBrush(hasFocus_? brushBodyH_ : brushBody_);
    }
    int b = penOutline_.width();
    p.drawRect(rect().adjusted(b-1,b-1,-2*b+1,-2*b+1));
}


void ClipWidget::enterEvent(QEvent *)
{
    hasFocus_ = true;
    update();
}

void ClipWidget::leaveEvent(QEvent *)
{
    hasFocus_ = false;
    update();
}



} // namespace GUI
} // namespace MO
