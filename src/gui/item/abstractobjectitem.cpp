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
#include <QGraphicsSimpleTextItem>
#include <QCursor>
#include <QDrag>
#include <QMessageBox>
#include <QUrl>
#include <QFileInfo>

#include "abstractobjectitem.h"
#include "objectgraphexpanditem.h"
#include "objectgraphconnectitem.h"
#include "gui/util/appicons.h"
#include "object/object.h"
#include "object/audioobject.h"
#include "object/objectfactory.h"
#include "object/audio/audiooutao.h"
#include "object/audio/audioinao.h"
#include "object/param/modulator.h"
#include "object/param/modulatorfloat.h"
#include "object/param/parameters.h"
#include "gui/util/objectgraphsettings.h"
#include "gui/util/objectgraphscene.h"
#include "gui/util/scenesettings.h"
#include "gui/util/objectmenu.h"
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
          minimumSize   (1, 1),
          itemExp       (0),
          itemName      (0),
          isMouseDown   (false),
          dragThresh    (false)
    { }

    void createConnectors();
    void updateConnectorPositions();
    void layoutChildItems();
    /** Helper to set the drag goal indicator from
        a LOCAL position */
    void setDragGoalPos(const QPointF& p);


    AbstractObjectItem * item; ///< parent item class
    Object * object;
    bool expanded, hover, dragHover, layouted, dragging;
    QPoint pos, //! pos in grid
            dragGoalPos; //! temp. drag gui indicator
    QSize size, //! size in grid coords
        unexpandedSize,
        minimumSize;
    QIcon icon;
    QPixmap iconPixmap;
    QBrush brushBack, brushBackSel, brushDragIndicator;
    ObjectGraphExpandItem * itemExp;
    QGraphicsSimpleTextItem * itemName;
    QList<ObjectGraphConnectItem*>
        inputItems,
        outputItems;

    bool isMouseDown,
         dragThresh;
    QPoint gridPosDown, dragThreshPos;
    QPointF posMouseDown;
};



// -------------------------- AbstractObjectItem ------------------------------

/** @todo when moving childs within a parent,
    expand top-left only after certain threshold distance */
AbstractObjectItem::AbstractObjectItem(Object *object, QGraphicsItem * parent)
    : QGraphicsItem     (parent),
      p_oi_             (new PrivateOI(this))
{
    MO_ASSERT(object, "no object given for AbstractObjectItem");

    // prepare private state
    p_oi_->object = object;
    // 'expanded' triangle item
    p_oi_->itemExp = new ObjectGraphExpandItem(this);
    setExpandVisible(false);
    p_oi_->itemExp->setPos(ObjectGraphSettings::penOutlineWidth() * 3.,
                           ObjectGraphSettings::penOutlineWidth() * 3.);
    updateColors();

    // setup QGraphicsItem
    setCursor(QCursor(Qt::SizeAllCursor));
    setAcceptHoverEvents(true);
    setAcceptDrops(true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemClipsChildrenToShape, true);
    setToolTip(object->name());

    // load item settings from object
    if (p_oi_->object->hasAttachedData(Object::DT_GRAPH_EXPANDED))
        p_oi_->expanded = p_oi_->object->getAttachedData(Object::DT_GRAPH_EXPANDED).toBool();
    else
        p_oi_->expanded = p_oi_->object->isAudioObject();

    p_oi_->createConnectors();
    p_oi_->layoutChildItems();
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
    // store gui state in object
    if (object())
        object()->setAttachedData(enable, Object::DT_GRAPH_EXPANDED);

    // set childs (in-)visible
    const auto list = childItems();
    for (auto i : list)
        if (i->type() >= T_BASE)
            i->setVisible(enable);

    p_oi_->layoutChildItems();
    setLayoutDirty();

    // bring me to front
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

void AbstractObjectItem::setExpandVisible(bool e)
{
    if (p_oi_->itemExp)
        p_oi_->itemExp->setVisible(e || true);
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
            setExpandVisible(true);

            // set visibility of new child
            item->setVisible(isExpanded());
        }

        if (isExpanded())
            setLayoutDirty();
    }
    else
    if (change == ItemChildRemovedChange)
    {
        bool isao = p_oi_->object && p_oi_->object->isAudioObject();

        // collapse if no child left
        if (!isao && isExpanded() && object()->childObjects().isEmpty()
                && !hasConnectors())
        {
            setExpanded(false);
            setExpandVisible(false);
        }
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

void AbstractObjectItem::PrivateOI::setDragGoalPos(const QPointF& p)
{
    auto np = item->mapToGrid(p);
    if (ObjectGraphScene * s = item->objectScene())
    {
        np = s->nextFreePosition(item, np);
        if (np != dragGoalPos)
        {
            dragGoalPos = np;
            item->update();
        }
    }
    else
        dragGoalPos = np;
}


void AbstractObjectItem::dragEnterEvent(QGraphicsSceneDragDropEvent * e)
{
    e->ignore();

//    for (const auto & i : e->mimeData()->formats())
//        MO_PRINT(i);

    // filename(s)
    if (e->mimeData()->hasUrls())
    {
        e->accept();
    }

    // drop of real object
    if (e->mimeData()->hasFormat(ObjectMimeData::mimeTypeString))
    {
        // avoid self-drop
        auto data = static_cast<const ObjectMimeData*>(e->mimeData());
        if (data->getDescription().pointer() == object())
        {
            e->ignore();
            return;
        }

        e->accept();
    }

    // drop of object from toolbar
    if (e->mimeData()->hasFormat(ObjectMenu::NewObjectMimeType))
    {
        // get object type
        auto classn = QString::fromUtf8(e->mimeData()->data(ObjectMenu::NewObjectMimeType));
        int typ = ObjectFactory::typeForClass(classn);
        if (typ < 0)
            return;
        // check if dropable
        if (object()->canHaveChildren(Object::Type(typ)))
            e->accept();
    }

    // drag indicators
    if (e->isAccepted())
    {
        p_oi_->dragHover = true;
        p_oi_->setDragGoalPos(e->pos());
        update();
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

void AbstractObjectItem::dragMoveEvent(QGraphicsSceneDragDropEvent * e)
{
    // update drag goal indicator
    if (p_oi_->dragHover)
        p_oi_->setDragGoalPos(e->pos());
}


void AbstractObjectItem::dropEvent(QGraphicsSceneDragDropEvent * e)
{
    e->ignore();
    p_oi_->dragHover = false;
    update(); // remove drag indicators

    // filename(s)
    if (e->mimeData()->hasUrls())
    {
        const auto list = e->mimeData()->urls();
        QList<Object*> objs;
        QList<QString> notworking;
        for (const QUrl& url : list)
        {
            QString shortfn = QFileInfo(url.toString()).fileName();
            try
            {
                if (auto o = ObjectFactory::createObjectFromUrl(url))
                    objs << o;
                else
                    notworking << QObject::tr("%1: format not supported")
                                  .arg(shortfn);
            }
            catch (const Exception& e)
            {
                notworking << QObject::tr("%1: %2")
                              .arg(shortfn)
                              .arg(e.what());
            }
        }
        // add them all at once
        if (!objs.isEmpty())
            objectScene()->addObjects(object(), objs, mapToGrid(e->pos()));
        // print errors
        if (!notworking.isEmpty())
        {
            QString msg = QObject::tr("The following urls could not be wrapped into an object.");
            for (const auto & text : notworking)
            {
                msg.append("\n" + text);
            }
            QMessageBox::information(0, QObject::tr("File drop"), msg);
        }
    }

    // drop of real object
    if (e->mimeData()->formats().contains(ObjectMimeData::mimeTypeString))
    {

        // construct a wrapper
        auto data = static_cast<const ObjectMimeData*>(e->mimeData());
        auto desc = data->getDescription();

        // analyze further
        if (!desc.isSameApplicationInstance())
        {
            QMessageBox::information(0,
                                     QMessageBox::tr("drop object"),
                                     QMessageBox::tr("Can't drop an object from another application instance."));
            return;
        }

        if (desc.pointer() && objectScene())
            objectScene()->popupObjectDrag(desc.pointer(), object(), e->scenePos());

        e->accept();
        return;
    }

    // drop of object from toolbar
    if (e->mimeData()->hasFormat(ObjectMenu::NewObjectMimeType))
    {
        // get object type
        auto classn = QString::fromUtf8(e->mimeData()->data(ObjectMenu::NewObjectMimeType));
        int typ = ObjectFactory::typeForClass(classn);
        // check if dropable
        if (typ < 0 || !object()->canHaveChildren(Object::Type(typ)))
            return;

        objectScene()->addObject(object(),
                                 ObjectFactory::createObject(classn),
                                 mapToGrid(e->pos())
                                 //mapToGrid(e->scenePos()) - gridPos()
                                 );

        e->accept();
        return;
    }
}

void AbstractObjectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    p_oi_->hover = true;
    update();
}

void AbstractObjectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    p_oi_->hover = false;
    p_oi_->dragHover = false;
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
            // start drag object (not position)
            if (e->modifiers() & Qt::SHIFT)
            {
                auto drag = new QDrag(scene());
                auto data = new ObjectMimeData();
                data->setObject(object());
                drag->setMimeData(data);
                drag->setPixmap(p_oi_->icon.pixmap(48, 48));
                drag->exec(Qt::LinkAction);
                return;
            }

            // start drag position
            p_oi_->dragging = true;
            if (!isSelected())
                setSelected(true);
        }
    }

    // drag position
    if (p_oi_->dragging)
    {
        const QPointF p = mapToParent(e->pos());
        QPoint newGrid = mapToGrid(p) - p_oi_->gridPosDown;

        if (newGrid != gridPos())
        {
            // check if space is free
            // XXX not working for moving expanded objects
            /// @todo fix check for overlapping objects when moving in scene editor
            auto it = sc->objectItemAt(parentObjectItem(),
                                       newGrid /*+ (parentObjectItem()
                                       ? parentObjectItem()->globalGridPos()
                                       : QPoint(0,0))*/);
            if (it == 0 || it == this || object()->hasParentObject(it->object()))
            {
                if ((newGrid.x() == 0 || newGrid.y() == 0))
                {
                    if (!p_oi_->dragThresh)
                    {
                        p_oi_->dragThresh = true;
                        return;
                    }
                    if (newGrid.x() < 0 || newGrid.y() < 0)
                    {
                        p_oi_->dragThresh = false;
                    }
                        else return;
                }
                /*
                if ((newGrid.x() < 1 || newGrid.y() < 1))
                {
                    if (!p_oi_->dragThresh)
                    {
                        p_oi_->dragThresh = true;
                        p_oi_->dragThreshPos = newGrid;
                        return;
                    }
                    else
                    {
                        if (newGrid == p_oi_->dragThreshPos)
                            return;
                    }
                }*/
                setGridPos(newGrid, true);
            }
        }

    }
}

void AbstractObjectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    QGraphicsItem::mouseReleaseEvent(e);

    p_oi_->isMouseDown = false;
    p_oi_->dragging = false;
    p_oi_->dragThresh = false;
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


QSize AbstractObjectItem::gridSize() const
{
#if 0
    QSize s = isExpanded() ? p_oi_->size : p_oi_->unexpandedSize;
    return QSize(std::max(s.width(),  p_oi_->minimumSize.width()),
                 std::max(s.height(), p_oi_->minimumSize.height()));
#else
    if (isExpanded())
        return QSize(std::max(p_oi_->size.width(),  p_oi_->minimumSize.width()),
                     std::max(p_oi_->size.height(), p_oi_->minimumSize.height()));
    else
        return p_oi_->unexpandedSize;
#endif
}

void AbstractObjectItem::setGridPos(const QPoint &pos1, bool expand)
{
    // limit to positive parent area
    QPoint pos(pos1);
    if (expand && parentObjectItem())
    {
        if (pos.x() < 1 || pos.y() < 1)
            parentObjectItem()->expandTopLeft(1-pos.x(), 1-pos.y());
        //pos = QPoint(std::max(1, pos1.x()), std::max(1, pos1.y()));
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

    p_oi_->layoutChildItems();

    if (isExpanded())
    {
        setLayoutDirty();
        update();
    }
}

// --------------------------- global queries ----------------------------------------------------

QList<AbstractObjectItem*> AbstractObjectItem::childObjectItems() const
{
    QList<AbstractObjectItem*> ret;
    auto l = childItems();
    for (auto c : l)
        if (c->type() >= AbstractObjectItem::T_BASE)
            ret << static_cast<AbstractObjectItem*>(c);
    return ret;
}

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

void AbstractObjectItem::updateColors()
{
    p_oi_->brushBack = ObjectGraphSettings::brushOutline(object());
    p_oi_->brushBackSel = ObjectGraphSettings::brushOutline(object(), true);
    p_oi_->brushDragIndicator = p_oi_->brushBack;
    p_oi_->brushDragIndicator.setColor(QColor(255,255,255,100));
    p_oi_->icon = AppIcons::iconForObject(object(),
                                ObjectFactory::colorForObject(object()));
    p_oi_->iconPixmap = p_oi_->icon.pixmap(ObjectGraphSettings::iconSize());

    update();
}

/** @todo AudioIn module hides outputs if no system-audio-ins are defined.
    Generally, there needs to be an abstraction between system-ins/outs and
    the modules. */
void AbstractObjectItem::PrivateOI::createConnectors()
{
    // visible parameters / modulator inputs
    QList<Parameter*> params = object->params()->getVisibleGraphParameters();
    for (Parameter * p : params)
    {
        inputItems.append( new ObjectGraphConnectItem(p, item) );
    }

    // desired signal outputs
    auto map = object->getNumberOutputs();
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        for (uint i = 0; i < it.value(); ++i)
        {
            // dont show outputs of output AO
            if (it.key() == ST_AUDIO && qobject_cast<AudioOutAO*>(object))
                continue;

            outputItems.append(
                new ObjectGraphConnectItem(false, it.key(), i, object->getOutputName(it.key(), i), item) );
        }
    }


    // audio input items
    if (AudioObject * ao = qobject_cast<AudioObject*>(object))
    {
        //item->setUnexpandedSize(QSize(1, 1));

        if (!qobject_cast<AudioInAO*>(ao))
        {
            if (ao->numAudioInputs() >= 0)
                for (int i=0; i<ao->numAudioInputs(); ++i)
                    inputItems.append( new ObjectGraphConnectItem(
                                           true, ST_AUDIO, i, ao->getAudioInputName(i), item) );
            else
                inputItems.append( new ObjectGraphConnectItem(
                                       true, ST_AUDIO, 0, ao->getAudioInputName(0), item) );
        }
        /*
        if (!qobject_cast<AudioOutAO*>(ao))
        {
            for (uint i=0; i<ao->numAudioOutputs(); ++i)
                outputItems.append( new ObjectGraphConnectItem(false, i, ao->getAudioOutputName(i), item) );
        }*/

    }

    int numCon = std::max(inputItems.size(), outputItems.size());

    // set size accordingly to number of inputs
    minimumSize.setWidth(numCon > 0 ? 2 : 1);
    minimumSize.setHeight(1);
    if (numCon)
        minimumSize.rheight() += 1 + (numCon-1) / ObjectGraphSettings::connectorsPerGrid();

    updateConnectorPositions();
}

void AbstractObjectItem::PrivateOI::layoutChildItems()
{
    updateConnectorPositions();

    const int boarder = ObjectGraphSettings::penOutlineWidth() + 2;
    const auto
            r = item->boundingRect(),
            // inner space (right of expand item)
            rr = QRectF(r.left() + ObjectGraphSettings::gridSize().width() * 0.3,
                        r.top() + boarder,
                        r.right() - boarder,
                        r.bottom() - boarder);


    if (item->gridSize().width() > 1)
    {
        // name label
        if (!itemName)
        {
            itemName = new QGraphicsSimpleTextItem(object->name(), item);
            itemName->setFont(ObjectGraphSettings::fontName());
            itemName->setBrush(ObjectGraphSettings::brushText(object));
        }

        // center name label
        itemName->setPos(QPointF(
                            rr.left() + (rr.width() - itemName->boundingRect().width()) / 2,
                            rr.top()
                            ));

        itemName->setVisible(true);
    }
    // when not large enough
    else
    {
        if (itemName)
            itemName->setVisible(false);
    }
}

void AbstractObjectItem::updateLabels()
{
    if (p_oi_->itemName)
        p_oi_->itemName->setText(object()->name());

    p_oi_->layoutChildItems();
}

void AbstractObjectItem::PrivateOI::updateConnectorPositions()
{
    const QRectF r(item->rect());

    qreal top = 0;

    if (item->gridSize().height() > 1)
        top = ObjectGraphSettings::gridSize().height();

    top += r.top();

    qreal heightfac = (r.height() - top) / std::max(1, inputItems.size());
    const bool vis = item->isExpanded();

    for (int i=0; i<inputItems.size(); ++i)
    {
        inputItems[i]->setPos(r.left(), top + (i + 0.5) * heightfac);
        inputItems[i]->setVisible(vis);
    }

    heightfac = (r.height() - top) / std::max(1, outputItems.size());

    for (int i=0; i<outputItems.size(); ++i)
    {
        outputItems[i]->setPos(r.right(), top + (i + 0.5) * heightfac);
        outputItems[i]->setVisible(vis);
    }

}

void AbstractObjectItem::updateConnectors()
{
    // clear previous items
    for (auto i : p_oi_->inputItems)
    {
        if (scene())
            scene()->removeItem(i);
        delete i;
    }
    p_oi_->inputItems.clear();

    for (auto i : p_oi_->outputItems)
    {
        if (scene())
            scene()->removeItem(i);
        delete i;
    }
    p_oi_->outputItems.clear();

    // create new
    p_oi_->createConnectors();

    p_oi_->layoutChildItems();
    setLayoutDirty();
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
            //if (static_cast<ObjectGraphConnectItem*>(c)->isAudioConnector())
                return static_cast<ObjectGraphConnectItem*>(c)->channel();
    return -1;
}

void AbstractObjectItem::expandTopLeft(int x, int y)
{
    if (x < 1 && y < 1)
        return;

    x = std::max(0, x);
    y = std::max(0, y);

    auto childs = childObjectItems();
    for (AbstractObjectItem * i : childs)
        i->setGridPos(i->gridPos() + QPoint(x, y));

    auto p = gridPos();
    setGridPos(QPoint(p.x() - x, p.y() - y), true);
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

    // grid size
    const auto s = ObjectGraphSettings::gridSize();

    // rect of children
    const auto crectf = childrenBoundingRect(true);
    // in grid coords
    QRect crect(std::round(crectf.left() / s.width()),
                std::round(crectf.top() / s.height()),
                std::round(crectf.width() / s.width()),
                std::round(crectf.height() / s.height())
                );

    /// @todo also shrink top-left corner

    const QSize gs = QSize(std::max(2, crect.right() + 1),
                           std::max(2, crect.bottom() + 1));
    setGridSize(gs);
}

#if 0
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
#endif
// --------------------------------------- shape and draw -----------------------------------------

bool AbstractObjectItem::hasConnectors() const
{
    return !p_oi_->inputItems.isEmpty()
            || !p_oi_->outputItems.isEmpty();
}

QPointF AbstractObjectItem::inputPos(uint c) const
{
    for (auto i : p_oi_->inputItems)
        if (//i->signalType() == ST_AUDIO &&
                i->channel() == c)
            return i->pos();

    QRectF r(rect());
    return QPointF(r.left(), r.top() + 4);
}


QPointF AbstractObjectItem::outputPos(uint c) const
{
    for (auto i : p_oi_->outputItems)
        if (//i->isAudioConnector() &&
                i->channel() == c)
            return i->pos();

    QRectF r(rect());
    return QPointF(r.right(), r.bottom() - 4);
}

QPointF AbstractObjectItem::inputPos(Parameter * p) const
{
    for (auto i : p_oi_->inputItems)
        if (i->isParameter() && i->parameter() == p)
            return i->pos();

    QRectF r(rect());
    return QPointF(r.left(), r.top() + 4);
}

QPointF AbstractObjectItem::outputPos(Modulator * m) const
{
//    if (m->isAudioToFloatConverter())
//        return outputPos(static_cast<ModulatorFloat*>(m)->channel());
    return outputPos(m->outputChannel());

//    QRectF r(rect());
//    return QPointF(r.right(), r.bottom() - 4);
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
    const qreal cornerRadius = (p_oi_->dragHover ? 0.25 : 0.1) *
                        ObjectGraphSettings::gridSize().width();

    p->drawRoundedRect(r, cornerRadius, cornerRadius);

    //p->setPen(QPen(Qt::green));
    //p->drawRect(childrenBoundingRect(true));

    //p_oi_->icon.paint(p, 0, 0,
    //                  ObjectGraphSettings::gridSize().width(),
    //                  ObjectGraphSettings::gridSize().height());
    const auto si = ObjectGraphSettings::gridSize() - ObjectGraphSettings::iconSize();
    p->drawPixmap(si.width()/2,
                  si.height()/2,
                  p_oi_->iconPixmap);

    // -- drag goal indicator --
    if (p_oi_->dragHover)
    {
        p->setPen(Qt::NoPen);
        p->setBrush(p_oi_->brushDragIndicator);
        int w = ObjectGraphSettings::gridSize().width();
        auto r = QRectF(w * p_oi_->dragGoalPos.x(),
                        w * p_oi_->dragGoalPos.y(),
                        w, w);
        p->drawRoundedRect(r, 0.1 * w, 0.1 * w);
    }
}


} // namespace GUI
} // namespace MO
