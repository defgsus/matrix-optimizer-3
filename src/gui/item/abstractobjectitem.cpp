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
#include <QGraphicsItemGroup>
#include <QCursor>
#include <QDrag>
#include <QMessageBox>

#include "abstractobjectitem.h"
#include "objectgraphexpanditem.h"
#include "objectgraphconnectitem.h"
#include "object/object.h"
#include "object/audioobject.h"
#include "object/objectfactory.h"
#include "object/audio/audiooutao.h"
#include "gui/util/objectgraphsettings.h"
#include "gui/util/objectgraphscene.h"
#include "gui/util/scenesettings.h"
#include "model/objectmimedata.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {

// ------------------------------ Private -------------------------------------

class AbstractObjectItem::PrivateOI
{
public:
    PrivateOI(AbstractObjectItem * item)
        : item          (item),
          object        (0),
          expanded      (false),
          hover         (false),
          dragHover     (false),
          layouted      (false),
          dragging      (false),
          size          (3, 3), // expanded size minimum
          unexpandedSize(1, 1),
          itemExp       (0),
          isMouseDown   (false)
    { }

    void updateConnectors();

    AbstractObjectItem * item; ///< parent item class
    Object * object;
    bool expanded, hover, dragHover, layouted, dragging;
    QPoint pos; ///< pos in grid
    QSize size, ///< size in grid coords
        unexpandedSize;
    QIcon icon;
    QPixmap iconPixmap;
    QBrush brushBack, brushBackSel;
    ObjectGraphExpandItem * itemExp;
    QList<ObjectGraphConnectItem*>
        inputItems,
        outputItems;

    bool isMouseDown;
    QPoint gridPosDown;
    QPointF posMouseDown;
};



// -------------------------- AbstractObjectItem ------------------------------


AbstractObjectItem::AbstractObjectItem(Object *object, QGraphicsItem * parent)
    : QGraphicsItem     (parent),
      p_oi_             (new PrivateOI(this))
{
    MO_ASSERT(object, "no object given for AbstractObjectItem");

    // prepare private state
    p_oi_->object = object;
    p_oi_->brushBack = ObjectGraphSettings::brushOutline(object);
    p_oi_->brushBackSel = ObjectGraphSettings::brushOutline(object, true);
    p_oi_->icon = ObjectFactory::iconForObject(p_oi_->object,
                                ObjectFactory::colorForObject(object));
    p_oi_->iconPixmap = p_oi_->icon.pixmap(ObjectGraphSettings::iconSize());
    p_oi_->itemExp = new ObjectGraphExpandItem(this);
    p_oi_->itemExp->setVisible(false);
    p_oi_->itemExp->setPos(ObjectGraphSettings::penOutlineWidth() * 3.,
                           ObjectGraphSettings::penOutlineWidth() * 3.);

    // setup QGraphicsItem
    setCursor(QCursor(Qt::SizeAllCursor));
    setAcceptHoverEvents(true);
    setAcceptDrops(true);
    setFlag(ItemIsSelectable, true);
    setToolTip(object->name());

    // input/output items
    if (AudioObject * ao = qobject_cast<AudioObject*>(object))
    {
        setUnexpandedSize(QSize(1, 2));
        if (ao->numAudioInputs() >= 0)
            for (int i=0; i<ao->numAudioInputs(); ++i)
                p_oi_->inputItems.append( new ObjectGraphConnectItem(i, ao->getInputName(i), this) );
        else
            p_oi_->inputItems.append( new ObjectGraphConnectItem(0, ao->getInputName(0), this) );

        if (!qobject_cast<AudioOutAO*>(ao))
        {
            for (uint i=0; i<ao->numAudioOutputs(); ++i)
                p_oi_->outputItems.append( new ObjectGraphConnectItem(i, this) );
        }

        p_oi_->updateConnectors();
    }
}

AbstractObjectItem::~AbstractObjectItem()
{
    delete p_oi_;
}

// -------------------------- state ----------------------------------

ObjectGraphScene * AbstractObjectItem::objectScene() const
{
    return qobject_cast<ObjectGraphScene*>(scene());
}

Object * AbstractObjectItem::object() const
{
    return p_oi_->object;
}

const QIcon& AbstractObjectItem::icon() const
{
    return p_oi_->icon;
}

AbstractObjectItem * AbstractObjectItem::parentObjectItem() const
{
    return parentItem() && parentItem()->type() >= T_BASE
            ? static_cast<AbstractObjectItem*>(parentItem())
            : 0;
}

bool AbstractObjectItem::isExpanded() const
{
    return p_oi_->expanded;
}

bool AbstractObjectItem::isHover() const
{
    return p_oi_->hover;
}

void AbstractObjectItem::setUnexpandedSize(const QSize & s)
{
    prepareGeometryChange();
    p_oi_->unexpandedSize = s;
}

void AbstractObjectItem::setExpanded(bool enable)
{
    if (p_oi_->expanded == enable)
        return ;

    prepareGeometryChange();

    // set state
    p_oi_->expanded = enable;
    // store in gui settings
    if (object())
        object()->setAttachedData(enable, Object::DT_GRAPH_EXPANDED);

    // set childs (in-)visible
    const auto list = childItems();
    for (auto i : list)
        if (i->type() >= T_BASE)
            i->setVisible(enable);

    setLayoutDirty();
    p_oi_->updateConnectors();

    // bring to front
    if (enable)
        if (auto s = objectScene())
            s->toFront(this);
}

bool AbstractObjectItem::isLayouted() const
{
    return p_oi_->layouted;
}

void AbstractObjectItem::setLayoutDirty(bool dirty)
{
    p_oi_->layouted = !dirty;
    // propagate dirty state up
    if (dirty)
        if (auto p = parentObjectItem())
            p->setLayoutDirty();
}

// --------------------------- events ---------------------------------

QVariant AbstractObjectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{

    if (change == ItemPositionChange)
    {
        // value is the new position.
        QPointF pos = value.toPointF();
        p_oi_->pos = mapToGrid(pos);
        setLayoutDirty();
    }
    else
    if (change == ItemChildAddedChange)
    {
        auto item = value.value<QGraphicsItem*>();
        if (item && item->type() >= T_BASE)
        {
            if (p_oi_->itemExp)
                p_oi_->itemExp->setVisible(true);

            // set visibility of new child
            item->setVisible(isExpanded());
        }

        if (isExpanded())
            setLayoutDirty();
    }

    QVariant ret = QGraphicsItem::itemChange(change, value);

    // change of selection must update connected modulator paths
    if (change == ItemSelectedChange)
    {
        auto s = objectScene();
        if (s)
        {
            s->repaintModulators(this);
        }
    }

    return ret;
}


void AbstractObjectItem::dragEnterEvent(QGraphicsSceneDragDropEvent * e)
{
    if (e->mimeData()->formats().contains(ObjectMimeData::mimeTypeString))
    {
        // avoid self-drop
        auto data = static_cast<const ObjectMimeData*>(e->mimeData());
        if (data->getDescription().pointer() == object())
            return;

        p_oi_->dragHover = true;
        update();
        e->accept();
    }
}

void AbstractObjectItem::dragLeaveEvent(QGraphicsSceneDragDropEvent * )
{
    if (p_oi_->dragHover)
    {
        p_oi_->dragHover = false;
        update();
    }
}


void AbstractObjectItem::dropEvent(QGraphicsSceneDragDropEvent * e)
{
    // analyze mime data
    if (!e->mimeData()->formats().contains(ObjectMimeData::mimeTypeString))
        return;

    // construct a wrapper
    auto data = static_cast<const ObjectMimeData*>(e->mimeData());
    auto desc = data->getDescription();

    // analyze further
    if (!desc.isFromSameApplicationInstance())
    {
        QMessageBox::information(0,
                                 QMessageBox::tr("drop object"),
                                 QMessageBox::tr("Can't drop an object from another application instance."));
        return;
    }

    if (desc.pointer() && objectScene())
        objectScene()->popupObjectDrag(desc.pointer(), object(), e->scenePos());
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
    if (e->button() == Qt::RightButton)
    {
        if (auto s = objectScene())
        {
            if (!(e->modifiers() && Qt::CTRL))
                    s->clearSelection();
            setSelected(true);
            s->toFront(this);
            s->popup(mapToGrid(e->scenePos()));
            e->accept();
            return;
        }
    }

    if (e->button() == Qt::LeftButton)
    {
        // store current state on click
        p_oi_->isMouseDown = true;
        p_oi_->posMouseDown = e->pos();//mapToParent(e->pos());
        p_oi_->gridPosDown = mapToGrid(e->pos());

        if (auto s = objectScene())
        {
            s->toFront(this);
            if (object() && !(e->modifiers() && Qt::CTRL))
                emit s->objectSelected(object());
        }

        e->accept();
        update();

        return;
    }
}

void AbstractObjectItem::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    //QGraphicsItem::mouseMoveEvent(e); // probably doesn't do anything

    auto sc = objectScene();
    if (!sc)
        return;

    // start dragging ?
    if (p_oi_->isMouseDown && !p_oi_->dragging)
    {
        if ((e->pos() - p_oi_->posMouseDown).manhattanLength() > 4)
        {
            // drag object id (not position)
            if (e->modifiers() & Qt::CTRL)
            {
                auto drag = new QDrag(scene());
                auto data = new ObjectMimeData();
                data->setObject(object());
                drag->setMimeData(data);
                drag->setPixmap(p_oi_->icon.pixmap(48, 48));
                drag->exec(Qt::CopyAction);
                return;
            }

            p_oi_->dragging = true;
        }
    }

    // drag position
    if (p_oi_->dragging)
    {
        const QPointF p = mapToParent(e->pos());
        QPoint newGrid = mapToGrid(p) - p_oi_->gridPosDown;

        if (parentObjectItem())
        {
            newGrid.rx() = std::max(1, newGrid.x());
            newGrid.ry() = std::max(1, newGrid.y());
        }

        if (newGrid != gridPos())
        {
            // check if space is free
            // XXX not working for big objects
            auto it = sc->objectItemAt(newGrid + (parentObjectItem()
                                       ? parentObjectItem()->globalGridPos()
                                       : QPoint(0,0)));
            if (it == 0 || it == this || it == parentObjectItem())
                sc->setGridPos(this, newGrid);
        }

    }
}

void AbstractObjectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    QGraphicsItem::mouseReleaseEvent(e);

    p_oi_->isMouseDown = false;
    p_oi_->dragging = false;
    update();
}


// --------------------------------------- coordinates -----------------------------------------


QPoint AbstractObjectItem::mapToGrid(const QPointF & f) const
{
    const auto s = ObjectGraphSettings::gridSize();
    const int ox = f.x() < 0 ? -1 : 0,
              oy = f.y() < 0 ? -1 : 0;
    return QPoint(int(f.x() / s.width()) + ox,
                  int(f.y() / s.height()) + oy);
}

QPointF AbstractObjectItem::mapFromGrid(const QPoint & p) const
{
    const auto s = ObjectGraphSettings::gridSize();
    return QPointF(p.x() * s.width(),
                   p.y() * s.height());
}

const QPoint& AbstractObjectItem::gridPos() const
{
    return p_oi_->pos;
}

QPoint AbstractObjectItem::globalGridPos() const
{
    auto p = parentObjectItem();
    return p ? p_oi_->pos + p->globalGridPos() : p_oi_->pos;
}


const QSize& AbstractObjectItem::gridSize() const
{
    return isExpanded() ? p_oi_->size : p_oi_->unexpandedSize;
}

void AbstractObjectItem::setGridPos(const QPoint &pos1)
{
    // limit to positive parent area
    QPoint pos(pos1);
    if (parentObjectItem())
    {
        pos = QPoint(std::max(1, pos1.x()), std::max(1, pos1.y()));
    }

    // no change?
    if (pos == p_oi_->pos)
        return;

    // tell QGraphicsScene
    prepareGeometryChange();
    // save in item
    p_oi_->pos = pos;
    // save in object
    if (p_oi_->object)
        p_oi_->object->setAttachedData(pos, Object::DT_GRAPH_POS);
    // set actual item position
    setPos(mapFromGrid(pos));
    // request layouter
    setLayoutDirty();
}

void AbstractObjectItem::setGridSize(const QSize &size)
{
    if (size == p_oi_->size)
        return;

    if (isExpanded())
        prepareGeometryChange();

    p_oi_->size = size;

    p_oi_->updateConnectors();

    if (isExpanded())
    {
        setLayoutDirty();
        update();
    }
}

// --------------------------- global queries ----------------------------------------------------

AbstractObjectItem * AbstractObjectItem::childItemAt(const QPoint& pos) const
{
    // pos is local in parent

    if (pos.x() < 0 || pos.y() < 0
        || pos.x() >= gridSize().width()
        || pos.y() >= gridSize().height())
        return 0;

    const auto list = childItems();
    for (auto i : list)
    if (i->type() >= AbstractObjectItem::T_BASE)
    {
        auto o = static_cast<AbstractObjectItem*>(i);

        if (auto ret = o->childItemAt(pos - o->gridPos()))
            return ret;
    }

    return 0;
}


// --------------------------------------- layout ---------------------------------------------------

void AbstractObjectItem::PrivateOI::updateConnectors()
{
    for (int i=0; i<inputItems.size(); ++i)
        inputItems[i]->setPos(item->inputPos(i) + QPointF(2,0));

    for (int i=0; i<outputItems.size(); ++i)
        outputItems[i]->setPos(item->outputPos(i) - QPointF(2,0));
}

QRectF AbstractObjectItem::childrenBoundingRect(bool checkVisibilty)
{
    if (!checkVisibilty)
        return childrenBoundingRect();

    QRectF rect;
    const auto list = childItems();
    for (QGraphicsItem * c : list)
    if (c->isVisible() && c->type() >= T_BASE)
    {
        rect |= c->mapToParent(c->boundingRect()).boundingRect();
    }
    return rect;
}

int AbstractObjectItem::channelForPosition(const QPointF &localPos)
{
    const auto list = childItems();
    for (QGraphicsItem * c : list)
    if (c->isVisible() && c->type() == ObjectGraphConnectItem::Type)
        if (c->shape().contains(localPos - c->pos()))
            return static_cast<ObjectGraphConnectItem*>(c)->channel();
    return -1;
}

void AbstractObjectItem::adjustSizeToChildren()
{
    // first do children
    const auto list = childItems();
    for (auto c : list)
    if (c->type() >= T_BASE)
    {
        auto o = static_cast<AbstractObjectItem*>(c);
        if (o->isExpanded())
            o->adjustSizeToChildren();
    }

    // rect of children
    const auto crect = childrenBoundingRect(true);
    // grid size
    const auto s = ObjectGraphSettings::gridSize();
    const QSize gs = QSize(std::max(2.0, crect.right() / s.width() + 1),
                           std::max(2.0, crect.bottom() / s.height() + 1));
    setGridSize(gs);
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
        if (i != this && i->type() >= T_BASE && i->pos().x() >= pos().x())
                list << static_cast<AbstractObjectItem*>(i);

    // move them away
    //const auto ofs = p_oi_->offset;
    MO_DEBUG("--");
    for (AbstractObjectItem * o : list)
    {
        MO_DEBUG(o->object()->name());
        o->setGridPos(QPoint(gridPos().x() + gridSize().width() + 1,
                                o->gridPos().y()));
    }

    /*
    const QRectF r = rect();
    auto list = scene()->items(r.right()+1, r.top(), 1000000, r.height(),
                               Qt::IntersectsItemBoundingRect,
                               Qt::DescendingOrder);
    */
}

// --------------------------------------- shape and draw -----------------------------------------

QPointF AbstractObjectItem::inputPos(uint c) const
{
    const auto r = rect();
    if (AudioObject * ao = qobject_cast<AudioObject*>(object()))
        return QPointF(r.left(),
                       r.top() + (c + 0.5) * r.height()
                            / std::max(1, ao->numAudioInputs()));
    else
        return QPointF(r.left(), r.center().y());
}


QPointF AbstractObjectItem::outputPos(uint c) const
{
    const auto r = rect();
    if (AudioObject * ao = qobject_cast<AudioObject*>(object()))
        return QPointF(r.right(),
                       r.top() + (c + 0.5) * r.height()
                            / std::max(1u, ao->numAudioOutputs()));
    else
        return QPointF(r.right(), r.center().y());
}

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
    if (isHover() || p_oi_->dragHover)
        p->setBrush(p_oi_->brushBackSel);
    else
        p->setBrush(p_oi_->brushBack);

    p->setPen(ObjectGraphSettings::penOutline(object(), isSelected()));

    const auto r = rect();
    const qreal cornerRadius = (p_oi_->dragHover ? 0.01 : 0.1) *
                        ObjectGraphSettings::gridSize().width();

    p->drawRoundedRect(r, cornerRadius, cornerRadius);

    //p_oi_->icon.paint(p, 0, 0,
    //                  ObjectGraphSettings::gridSize().width(),
    //                  ObjectGraphSettings::gridSize().height());
    const auto si = ObjectGraphSettings::gridSize() - ObjectGraphSettings::iconSize();
    p->drawPixmap(si.width()/2,
                  si.height()/2,
                  p_oi_->iconPixmap);
}


} // namespace GUI
} // namespace MO
