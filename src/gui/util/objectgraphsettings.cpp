/** @file objectgraphsettings.cpp

    @brief Global settings for ObjectGraphView and it's items

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QPen>

#include "objectgraphsettings.h"
#include "gui/item/abstractobjectitem.h"
#include "object/objectfactory.h"


namespace MO {
namespace GUI {

// XXX link to application settings


class ObjectGraphSettings::Private
{
public:
    static QPainterPath
        * ppExpanded,
        * ppCollapsed;
};

QPainterPath * ObjectGraphSettings::Private::ppExpanded = 0;
QPainterPath * ObjectGraphSettings::Private::ppCollapsed = 0;

QSize ObjectGraphSettings::gridSize()
{
    //return QSize(64, 64);
    return QSize(54, 54);
}

QSize ObjectGraphSettings::iconSize()
{
    const auto s = gridSize();
    return QSize(s.width() - 12, s.height() - 12);
}

QSize ObjectGraphSettings::expandItemSize()
{
    return QSize(12, 12);
}

QBrush ObjectGraphSettings::brushBackground()
{
    return QBrush(QColor(30, 30, 30));
}

const QPainterPath& ObjectGraphSettings::pathExpanded()
{
    if (!Private::ppExpanded)
    {
        const auto s(expandItemSize());
        Private::ppExpanded = new QPainterPath(QPointF(0,0));
        Private::ppExpanded->lineTo(QPointF(s.width() / 2, s.height()));
        Private::ppExpanded->lineTo(QPointF(s.width(), 0));
        Private::ppExpanded->lineTo(QPointF(0, 0));
    }

    return *Private::ppExpanded;
}

const QPainterPath& ObjectGraphSettings::pathCollapsed()
{
    if (!Private::ppCollapsed)
    {
        const auto s(expandItemSize());
        Private::ppCollapsed = new QPainterPath(QPointF(0,0));
        Private::ppCollapsed->lineTo(QPointF(s.width(), s.height() / 2));
        Private::ppCollapsed->lineTo(QPointF(0, s.height()));
        Private::ppCollapsed->lineTo(QPointF(0, 0));
    }

    return *Private::ppCollapsed;
}

QPen ObjectGraphSettings::penOutline(const AbstractObjectItem * item)
{
    QPen pen;
    if (item->object())
        pen.setColor(ObjectFactory::colorForObject(item->object()).darker(130));
    pen.setWidth(penOutlineWidth());
    return pen;
}

int ObjectGraphSettings::penOutlineWidth()
{
    return 3;
}

} // namespace GUI
} // namespace MO
