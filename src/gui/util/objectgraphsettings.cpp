/** @file objectgraphsettings.cpp

    @brief Global settings for ObjectGraphView and it's items

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include "objectgraphsettings.h"

namespace MO {
namespace GUI {

// XXX link to application settings

QSize ObjectGraphSettings::gridSize()
{
    return QSize(64, 64);
}

QSize ObjectGraphSettings::expandItemSize()
{
    return QSize(12, 12);
}

QBrush ObjectGraphSettings::brushBackground()
{
    return QBrush(QColor(30, 30, 30));
}

} // namespace GUI
} // namespace MO
