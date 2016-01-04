/** @file objectgraphsettings.cpp

    @brief Global settings for ObjectGraphView and it's items

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QPen>

#include "objectgraphsettings.h"
#include "gui/item/abstractobjectitem.h"
#include "gui/item/objectgraphconnectitem.h"
#include "object/object.h"
#include "object/control/clip.h"
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

int ObjectGraphSettings::connectorsPerGrid()
{
    return gridSize().height() / 18;
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

QSize ObjectGraphSettings::controlItemSize()
{
    return QSize(16, 16);
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
        c = c.lighter(150);

    return c;
}


QPen ObjectGraphSettings::penOutline(const Object * o, bool sel)
{
    QPen pen(colorOutline(o, sel));
    pen.setWidth(penOutlineWidth());
    return pen;
}

QPen ObjectGraphSettings::penOutlineError(bool /*sel*/)
{
    QPen pen(Qt::red);
    pen.setWidth(penOutlineWidth()*2);
    return pen;
}

int ObjectGraphSettings::penOutlineWidth()
{
    return 3;
}

QBrush ObjectGraphSettings::brushOutline(const Object *o, bool selected)
{
    QColor c = penOutline(o).color().darker(400 - 70 * selected);
/*    if (o->type() == Object::T_CLIP)
        c = QColor::fromHsl(c.hslHue(), c.hslSaturation() / 4,
                            std::max(30, c.lightness()));
*/
    return QBrush(c);
}


QBrush ObjectGraphSettings::brushConnector(ObjectGraphConnectItem * i)
{
    int hue = -1;
    switch (i->signalType())
    {

        case ST_SELECT:
        case ST_TEXT:
        case ST_FILENAME:
        case ST_TIMELINE1D:
        case ST_CALLBACK:
        case ST_FONT:
            break;
        case ST_FLOAT:
        case ST_INT: hue = ObjectFactory::hueForObject(Object::T_SEQUENCE_FLOAT);
            break;
        case ST_TRANSFORMATION: hue = ObjectFactory::hueForObject(Object::T_TRANSFORMATION);
            break;
        case ST_TEXTURE: hue = ObjectFactory::hueForObject(Object::T_TEXTURE);
            break;
        case ST_AUDIO: hue = 0;
            break;
        case ST_GEOMETRY: hue = ObjectFactory::hueForObject(Object::T_GEOMETRY);
            break;
    }

    int sat = hue == -1 ? 0 : 100;
    int bright = 150;

    if (i->isHovered())
        bright += 50;

    return QBrush(QColor::fromHsl(hue, sat, bright));
}


QPen ObjectGraphSettings::penModulator(const Modulator * mod, bool highl, bool sel, bool active)
{
    int sat = 70 + highl * 110,
        bright = 150 + sel * 70,
        hue = 140;
    if (mod->modulator() && !mod->modulator()->isAudioObject())
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

QFont ObjectGraphSettings::fontConnector()
{
    QFont f;
    f.setPointSizeF(8);
    return f;
}



// ---------------------------- text ------------------------------

QColor ObjectGraphSettings::colorText(const Object * o)
{
    auto c = ObjectFactory::colorForObject(o);
    return QColor::fromHsl(c.hue(), c.saturation() / 2, 200);
}


QFont ObjectGraphSettings::fontName()
{
    QFont f;
    f.setPointSizeF(10);
    return f;
}




// ---------------------- modulators ------------------------------

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
    shape.lineTo(to + QPointF(-5, 5));
#endif
    shape.lineTo(to);

    return shape;
}

} // namespace GUI
} // namespace MO
