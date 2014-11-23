/** @file objectgraphsettings.h

    @brief Global settings for ObjectGraphView and it's items

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#ifndef MOSRC_GUI_OBJECTGRAPHSETTINGS_H
#define MOSRC_GUI_OBJECTGRAPHSETTINGS_H

#include <QSize>
#include <QBrush>

namespace MO {
namespace GUI {


class ObjectGraphSettings
{
public:

    static QSize gridSize();

    static QSize expandItemSize();

    static QBrush brushBackground();
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_OBJECTGRAPHSETTINGS_H
