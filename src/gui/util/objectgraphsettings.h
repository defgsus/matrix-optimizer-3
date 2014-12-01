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
#include <QPainterPath>

namespace MO {
class Modulator;
class Object;
class AudioObjectConnection;
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

    static QColor colorOutline(const Object *o, bool selected = false);
    static QPen penOutline(const Object *o, bool selected = false);
    static int penOutlineWidth();
    // stupid name but brushBackground() is already taken
    static QBrush brushOutline(const Object * o, bool selected = false);

    static QPen penModulator(const Modulator *, bool highlight = false, bool selected = false, bool active = true);
    static QPen penAudioConnection(const AudioObjectConnection*,
                                   bool highlight = false, bool selected = false, bool active = true);

    static QPen penSelectionFrame();

    static QPainterPath pathWire(const QPointF& from, const QPointF& to);

private:

    class Private;
    static Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_OBJECTGRAPHSETTINGS_H
