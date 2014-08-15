/** @file audiounitconnectorwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QPainter>

#include "audiounitconnectorwidget.h"
#include "object/audio/audiounit.h"
#include "object/modulatorobjectfloat.h"

namespace MO {
namespace GUI {


AudioUnitConnectorWidget::AudioUnitConnectorWidget
            (AudioUnit *au, uint channel, bool isInput, bool isAudio, QWidget *parent)
    : QWidget   (parent),
      unit_     (au),
      modFloat_ (0),
      channel_  (channel),
      isInput_  (isInput),
      isAudio_  (isAudio),
      hovered_  (false)
{
    if (isAudio)
    {
        setFixedSize(10, 10);
    }
    else
    {
        setFixedSize(40, 10);
        setMouseTracking(true);
    }

    brushAudio_ = QBrush(QColor(120,40,80));
    penAudio_ = QPen(brushAudio_.color().darker(150));

    brushMod_ = QBrush(QColor(0,90,90));
    penMod_ = QPen(brushMod_.color().darker(150));

    brushModHover_ = QBrush(brushMod_.color().lighter(150));
    penModHover_ = QPen(QColor(200,255,255));

    brushModValue_ = QBrush(brushMod_.color().lighter(200));

}

void AudioUnitConnectorWidget::enterEvent(QEvent *)
{
    hovered_ = true;
    update();
}

void AudioUnitConnectorWidget::leaveEvent(QEvent *)
{
    hovered_ = false;
    update();
}

void AudioUnitConnectorWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (isAudio_)
    {
        p.setBrush(brushAudio_);
        p.setPen(penAudio_);

        p.drawEllipse(rect().adjusted(0,0,-1,-1));
    }
    else
    {
        if (hovered_)
        {
            p.setBrush(brushModHover_);
            p.setPen(penModHover_);
        }
        else
        {
            p.setBrush(brushMod_);
            p.setPen(penMod_);
        }

        p.drawRect(rect().adjusted(0,0,-1,-1));

        // paint value
        if (modFloat_ && modFloat_->inputValue() > 0)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(brushModValue_);

            // XXX adjustable range??
            int x = std::min(width()-2, (int)(modFloat_->inputValue() * width() - 1));
            p.drawRect(1,1, x,height()-2);
        }
    }
}


} // namespace GUI
} // namespace MO
