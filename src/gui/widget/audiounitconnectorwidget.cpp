/** @file audiounitconnectorwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QPainter>

#include "audiounitconnectorwidget.h"

namespace MO {
namespace GUI {


AudioUnitConnectorWidget::AudioUnitConnectorWidget
            (AudioUnit *au, uint channel, bool isInput, bool isAudio, QWidget *parent)
    : QWidget   (parent),
      unit_     (au),
      channel_  (channel),
      isInput_  (isInput),
      isAudio_  (isAudio)
{
    setFixedSize(8, 8);

    colorAudioUnit_ = QColor(120,40,80);
    colorModulatorObject_ = QColor(0, 90, 90);
}


void AudioUnitConnectorWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (isAudio_)
    {
        p.setBrush(QBrush(colorAudioUnit_));
        p.setPen(QPen(colorAudioUnit_.darker(150)));

        p.drawEllipse(rect().adjusted(0,0,-1,-1));
    }
    else
    {
        p.setBrush(QBrush(colorModulatorObject_));
        p.setPen(QPen(colorModulatorObject_.darker(150)));

        p.drawRect(rect().adjusted(0,0,-1,-1));
    }
}


} // namespace GUI
} // namespace MO
