/** @file audiounitwidget.cpp

    @brief Widget for displaying/connecting AudioUnits

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include "audiounitwidget.h"

namespace MO {
namespace GUI {



AudioUnitWidget::AudioUnitWidget(AudioUnit * au, QWidget *parent) :
    QWidget     (parent),
    unit_       (au)
{
    setMaximumSize(120,120);

    setAutoFillBackground(true);
    QPalette pal(palette());
    pal.setColor(QPalette::Window, pal.color(QPalette::Window).darker(130));
    setPalette(pal);
}


} // namespace GUI
} // namespace MO
