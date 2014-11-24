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
class Modulator;
namespace GUI {

class AbstractObjectItem;


class ObjectGraphSettings
{
public:

    static QSize gridSize();
    static QSize iconSize();

    static QSize expandItemSize();

    static QBrush brushBackground();

    static const QPainterPath& pathExpanded();
    static const QPainterPath& pathCollapsed();

    static QPen penOutline(const AbstractObjectItem *, bool selected = false);
    static int penOutlineWidth();

    static QPen penModulator(const Modulator *, bool highlight = false, bool selected = false);

private:

    class Private;
    static Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_OBJECTGRAPHSETTINGS_H
