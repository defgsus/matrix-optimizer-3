/** @file objectgraphsettings.cpp

    @brief Global settings for ObjectGraphView and it's items

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QPen>

#include "objectgraphsettings.h"
#include "gui/item/abstractobjectitem.h"
#include "object/object.h"
#include "object/clip.h"
#include "object/audioobject.h"
#include "object/objectfactory.h"
#include "object/param/modulator.h"
#include "object/util/audioobjectconnections.h"

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

QColor ObjectGraphSettings::colorOutline(const Object * o, bool sel)
{
    QColor c = ObjectFactory::colorForObject(o).darker(140);
    if (o->type() == Object::T_CLIP)
        c = static_cast<const Clip*>(o)->color();

    if (sel)
        c = c.lighter(180);

    return c;
}

QPen ObjectGraphSettings::penOutline(const Object * o, bool sel)
{
    QPen pen(colorOutline(o, sel));
    pen.setWidth(penOutlineWidth());
    return pen;
}

int ObjectGraphSettings::penOutlineWidth()
{
    return 3;
}

QBrush ObjectGraphSettings::brushOutline(const Object *o, bool selected)
{
    QColor c = penOutline(o).color().darker(400 - 70 * selected);
    if (o->type() == Object::T_CLIP)
        c = QColor::fromHsl(c.hslHue(), c.hslSaturation() / 4,
                            std::max(30, c.lightness()));

    return QBrush(c);
}

QPen ObjectGraphSettings::penModulator(const Modulator * mod, bool highl, bool sel, bool active)
{
    int sat = 70 + highl * 110,
        bright = 150 + sel * 70,
        hue = 140;
    if (mod->modulator())
        hue = ObjectFactory::hueForObject(mod->modulator()->type());
    if (!active)
        sat /= 4;
    if (hue == -1)
        sat = 0;

    QPen pen(QColor::fromHsl(hue, sat, bright, active ? 196 : 64));
    pen.setWidth(2 + (highl ? 1 : 0));
    return pen;
}

QPen ObjectGraphSettings::penAudioConnection(const AudioObjectConnection * mod, bool highl, bool sel, bool active)
{
    int sat = 70 + highl * 110,
        bright = 150 + sel * 70,
        hue = 140;
    if (mod->from())
        hue = ObjectFactory::hueForObject(mod->from()->type());
    if (!active)
        sat /= 4;
    if (hue == -1)
        sat = 0;

    QPen pen(QColor::fromHsl(hue, sat, bright, active ? 196 : 64));
    pen.setWidth(2 + (highl ? 1 : 0));
    return pen;
}

QPen ObjectGraphSettings::penSelectionFrame()
{
    QPen p(Qt::DotLine);
    p.setColor(Qt::white);
    return p;
}






namespace {

    void addCubicPath(QPainterPath& shape, const QPointF &from, const QPointF &to, qreal forward = 1.0)
    {
        auto pd = to - from;
        shape.cubicTo(from.x() + forward * std::abs(pd.x()) / 2,    from.y(),
                      to.x() - forward * std::abs(pd.x()) / 2,      to.y(),
                      to.x(), to.y());
    }

}

QPainterPath ObjectGraphSettings::pathWire(const QPointF &from, const QPointF &to)
{
    QPainterPath shape(from);

    addCubicPath(shape, from, to);

    // arrow head
#if (0) // arrow head in linear direction of path
    const Vec2
            dir = glm::normalize(Vec2(pd.x(), pd.y())) * 10.f,
            p1 = MATH::rotate(dir, 140),
            p2 = MATH::rotate(dir, 220);
    shape.lineTo(to.x() + p1.x,
                 to.y() + p1.y);
    shape.lineTo(to.x() + p2.x,
                 to.y() + p2.y);
#else // arrow head
    shape.lineTo(to + QPointF(-5,-5));
    shape.lineTo(to + QPointF(-5,5));
#endif
    shape.lineTo(to);

    return shape;
}

} // namespace GUI
} // namespace MO
