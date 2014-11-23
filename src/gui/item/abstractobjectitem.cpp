/** @file abstractobjectitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QPainter>
#include <QIcon>
#include <QBrush>

#include "abstractobjectitem.h"
#include "object/objectfactory.h"
#include "gui/util/objectgraphsettings.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {

// ------------------------------ Private -------------------------------------

class AbstractObjectItem::PrivateOI
{
public:
    PrivateOI()
        : object    (0),
          expanded  (false),
          hover     (false),
          pos       (0, 0),
          size      (1, 1)
    { }

    Object * object;
    bool expanded, hover;
    QPoint pos;
    QSize size;
    QPixmap iconPixmap;
    QBrush brushBack;
};



// -------------------------- AbstractObjectItem ------------------------------


AbstractObjectItem::AbstractObjectItem(Object *object, QGraphicsItem * parent)
    : QGraphicsItem     (parent),
      p_oi_             (new PrivateOI())
{
    MO_ASSERT(object, "no object given for AbstractObjectItem");

    // prepare private state
    p_oi_->object = object;
    p_oi_->brushBack = ObjectGraphSettings::brushBackground();
    p_oi_->brushBack.setColor(p_oi_->brushBack.color().lighter(130));
    p_oi_->iconPixmap = icon().pixmap(ObjectGraphSettings::gridSize());

    setAcceptHoverEvents(true);
}

AbstractObjectItem::~AbstractObjectItem()
{
    delete p_oi_;
}

Object * AbstractObjectItem::object() const
{
    return p_oi_->object;
}

const QIcon& AbstractObjectItem::icon() const
{
    return ObjectFactory::iconForObject(p_oi_->object);
}

bool AbstractObjectItem::isExpanded() const
{
    return p_oi_->expanded;
}

bool AbstractObjectItem::isHover() const
{
    return p_oi_->hover;
}

void AbstractObjectItem::setExpanded(bool enable)
{
    if (p_oi_->expanded == enable)
        return ;

    prepareGeometryChange();
    p_oi_->expanded = enable;
}

const QPoint& AbstractObjectItem::gridPos() const
{
    return p_oi_->pos;
}

const QSize& AbstractObjectItem::gridSize() const
{
    static QSize unExpanded(1, 1);
    return isExpanded() ? p_oi_->size : unExpanded;
}

void AbstractObjectItem::setGridPos(const QPoint &pos)
{
    prepareGeometryChange();
    p_oi_->pos = pos;
}

void AbstractObjectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    p_oi_->hover = true;
    update();
}

void AbstractObjectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    p_oi_->hover = false;
    update();
}

QRectF AbstractObjectItem::boundingRect() const
{
    const auto pos = gridPos();
    const auto size = gridSize();
    const auto scale = ObjectGraphSettings::gridSize();

    return QRectF(pos.x() * scale.width(),
                  pos.y() * scale.height(),
                  size.width() * scale.width() - 1,
                  size.height() * scale.height() - 1);
}

void AbstractObjectItem::paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *)
{
    const auto pos = gridPos();
    const auto size = gridSize();
    const auto scale = ObjectGraphSettings::gridSize();

    if (isHover())
        p->setBrush(p_oi_->brushBack);
    else
        p->setBrush(Qt::NoBrush);
    p->setPen(QPen(QColor(255,255,255)));

    const int
            wi = size.width() * scale.width(),
            he = size.height() * scale.height();
    const qreal
            cornerRadius = 0.1 * std::min(wi, he);
    p->drawRoundedRect(pos.x() * scale.width(),
                       pos.y() * scale.height(),
                       wi, he,
                       cornerRadius, cornerRadius);

    //p->drawRoundedRect(boundingRect(), cornerRadius, cornerRadius);

    p->drawPixmap(pos.x() * scale.width(),
                  pos.y() * scale.height(), p_oi_->iconPixmap);
}


} // namespace GUI
} // namespace MO
