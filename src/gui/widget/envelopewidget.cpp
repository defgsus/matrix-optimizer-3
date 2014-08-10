/** @file envelopewidget.cpp

    @brief A display for a number of levels

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include <QPainter>
#include <QPaintEvent>

#include "envelopewidget.h"
#include "io/error.h"


namespace MO {
namespace GUI {


EnvelopeWidget::EnvelopeWidget(QWidget *parent) :
    QWidget             (parent),
    numChannels_        (0),
    doAnimate_          (false)
{
    setMinimumSize(120, 40);
}

void EnvelopeWidget::setNumberChannels(uint num)
{
    level_.resize(num);
    numChannels_ = num;
}

void EnvelopeWidget::setLevel(uint index, F32 value)
{
    MO_ASSERT(index < level_.size(), "");
    level_[index] = value;
}

void EnvelopeWidget::setLevel(const F32 *levels)
{
    for (uint i=0; i<level_.size(); ++i)
        level_[i] = *levels++;
}

void EnvelopeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setPen(Qt::NoPen);
    QBrush black = QBrush(QColor(0,0,0));
    QBrush white = QBrush(QColor(70,250,70));
    QBrush red = QBrush(QColor(255,0,0));

    if (numChannels_ == 0)
    {
        p.setBrush(black);
        p.drawRect(rect());
        return;
    }

    int w = width() / numChannels_;

    for (uint i=0; i<numChannels_; ++i)
    {
        float h = height() - 1 - level_[i] * height();
        p.setBrush(black);
        p.drawRect(QRect(i*w, 0, w-1, h));
        p.setBrush(level_[i] >= 1 ? red : white);
        p.drawRect(QRect(i*w, h, w-1, height()-h));
    }

    if (doAnimate_)
        update();
}

} // namespace GUI
} // namespace MO
