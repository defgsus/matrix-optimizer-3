/** @file abstractobjectitem.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QDebug>

#include <QPainter>
#include <QIcon>
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

#include "abstractobjectitem.h"
#include "objectgraphexpanditem.h"
#include "object/object.h"
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
        : object        (0),
          expanded      (false),
          hover         (false),
          size          (2, 2), // expanded size minimum
          isMouseDown   (false)
    { }

    Object * object;
    bool expanded, hover;
    QPoint pos, offset; ///< pos in grid
    QSize size; ///< size in grid coords
    QPixmap iconPixmap;
    QBrush brushBack;
    ObjectGraphExpandItem * itemExp;

    bool isMouseDown;
    QPoint gridPosDown_;
    QPointF posMouseDown_;
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
    p_oi_->itemExp = new ObjectGraphExpandItem(this);
    p_oi_->itemExp->setPos(ObjectGraphSettings::penOutlineWidth() * 3.,
                           ObjectGraphSettings::penOutlineWidth() * 3.);

    // setup graphicsItems
    setCursor(QCursor(Qt::SizeAllCursor));
    setAcceptHoverEvents(true);

}

AbstractObjectItem::~AbstractObjectItem()
{
    delete p_oi_;
}

// -------------------------- state ----------------------------------

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


// --------------------------- events ---------------------------------

QVariant AbstractObjectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene())
    {
        // value is the new position.
        QPointF pos = value.toPointF();
        p_oi_->pos = mapToGrid(pos);
    }

    return QGraphicsItem::itemChange(change, value);
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


void AbstractObjectItem::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    // store current state on click
    if (e->button() == Qt::LeftButton)
    {
        p_oi_->isMouseDown = true;
        p_oi_->posMouseDown_ = mapToScene(e->pos());
        p_oi_->gridPosDown_ = gridPos();

        e->accept();
        update();
    }
}

void AbstractObjectItem::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    if (!scene())
        return;

    // drag position
    if (p_oi_->isMouseDown)
    {
        const QPointF p = mapToScene(e->pos());
        QPoint newGrid = mapToGrid(p);

        //if (newGrid != gridPos() &&
        //        !itemInGrid(newGrid))
        {
            setGridPos(newGrid);
            //ensureVisible(QRectF(), 0,0);
        }

    }
}

void AbstractObjectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * )
{
    p_oi_->isMouseDown = false;
    update();
}


// --------------------------------------- coordinates -----------------------------------------


QPoint AbstractObjectItem::mapToGrid(const QPointF & f) const
{
    const auto s = ObjectGraphSettings::gridSize();
    const int ox = f.x() < 0 ? -1 : 0,
              oy = f.y() < 0 ? -1 : 0;
    return QPoint(int(f.x() / s.width()) + ox,
                  int(f.y() / s.height()) + oy)
            - p_oi_->offset;
}

QPointF AbstractObjectItem::mapFromGrid(const QPoint & p) const
{
    const auto s = ObjectGraphSettings::gridSize();
    const int ox = p.x() < 0 ? 0 : 0,
              oy = p.y() < 0 ? 0 : 0;
    return QPointF((p.x() + ox + p_oi_->offset.x()) * s.width(),
                   (p.y() + oy + p_oi_->offset.y()) * s.height());
}

const QPoint& AbstractObjectItem::gridPos() const
{
    return p_oi_->pos;
}


void AbstractObjectItem::setGridOffset(const QPoint &offset)
{
    p_oi_->offset = offset;
    update();
}

const QSize& AbstractObjectItem::gridSize() const
{
    static QSize unExpanded(1, 1);
    return isExpanded() ? p_oi_->size : unExpanded;
}

void AbstractObjectItem::setGridPos(const QPoint &pos)
{
    prepareGeometryChange();
    setPos(mapFromGrid(pos));
    adjustRightItems();
}

// --------------------------- global queries ----------------------------------------------------

// XXX Does not seem to work right
AbstractObjectItem * AbstractObjectItem::itemInGrid(const QPoint& p) const
{
    if (!scene())
        return 0;

    const QSize s = ObjectGraphSettings::gridSize();
    const QPointF f(mapFromGrid(p));
    const QList<QGraphicsItem*> list = scene()->items(
                QRectF(f.x(), f.y(), s.width(), s.height()));
    for (auto i : list)
        if (auto item = dynamic_cast<AbstractObjectItem*>(i))
            return item;
    return 0;
}

void AbstractObjectItem::adjustRightItems()
{
    if (!scene())
        return;

    // get colliding items
    auto list2 = scene()->collidingItems(this, Qt::IntersectsItemBoundingRect);

    if (list2.isEmpty())
        return;

    // get all object-items that should be moved right
    QList<AbstractObjectItem*> list;
    for (QGraphicsItem * i : list2)
        if (i != this && i->pos().x() >= pos().x())
            if (auto o = dynamic_cast<AbstractObjectItem*>(i))
                list << o;

    // move them away
    //const auto ofs = p_oi_->offset;
    MO_DEBUG("--");
    for (AbstractObjectItem * o : list)
        MO_DEBUG(o->object()->name());
//        o->setGridPos(QPoint(gridPos().x() + gridSize().width() + 1,
//                                o->gridPos().y()));


    /*
    const QRectF r = rect();
    auto list = scene()->items(r.right()+1, r.top(), 1000000, r.height(),
                               Qt::IntersectsItemBoundingRect,
                               Qt::DescendingOrder);
    */
}

// --------------------------------------- shape and draw -----------------------------------------

QRectF AbstractObjectItem::rect() const
{
    const auto size = gridSize();
    const auto scale = ObjectGraphSettings::gridSize();

    const qreal
            pw = ObjectGraphSettings::penOutlineWidth()
                + (isExpanded() ? 0 : (ObjectGraphSettings::penOutlineWidth() + 1));

    return QRectF(pw, pw, size.width() * scale.width() - pw*2 - 1,
                          size.height() * scale.height() - pw*2 - 1);
}

QRectF AbstractObjectItem::boundingRect() const
{
    const auto r = rect();

    const qreal pw = ObjectGraphSettings::penOutlineWidth();
    return r.adjusted(-pw,-pw, pw, pw);
}

void AbstractObjectItem::paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (isHover())
        p->setBrush(p_oi_->brushBack);
    else
        p->setBrush(Qt::NoBrush);

    p->setPen(ObjectGraphSettings::penOutline(this));

    const auto r = rect();
    const qreal cornerRadius = 0.1 * ObjectGraphSettings::gridSize().width();

    p->drawRoundedRect(r, cornerRadius, cornerRadius);

    p->drawPixmap(0, 0, p_oi_->iconPixmap);
}


} // namespace GUI
} // namespace MO
