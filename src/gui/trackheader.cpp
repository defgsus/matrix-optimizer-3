/** @file trackheader.cpp

    @brief Track/Names area working with TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#include "trackheader.h"

namespace MO {
namespace GUI {

TrackHeader::TrackHeader(QWidget *parent) :
    QWidget(parent)
{
    setMinimumSize(100,240);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    QPalette p(palette());
    p.setColor(QPalette::Background, QColor(50,50,50));
    setPalette(p);
    setAutoFillBackground(true);
}



} // namespace GUI
} // namespace MO
