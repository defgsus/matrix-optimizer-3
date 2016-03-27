/** @file objectgraphscene.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#include <map>

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMessageBox>
#include <QFileInfo>
#include <QUrl>

#include "objectgraphscene.h"
#include "gui/item/abstractobjectitem.h"
#include "gui/item/audioconnectionitem.h"
#include "gui/item/modulatoritem.h"
#include "gui/item/objectgraphconnectitem.h"
#include "gui/geometrydialog.h"
#include "gui/modulatordialog.h"
#include "gui/texteditdialog.h"
#include "gui/evolutiondialog.h"
#include "gui/util/objectgraphsettings.h"
#include "gui/util/objectmenu.h"
#include "gui/util/appicons.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/audioobject.h"
#include "object/visual/model3d.h"
#include "object/visual/geometryobject.h"
#include "object/texture/textureobjectbase.h"
#include "object/util/objectfactory.h"
#include "object/control/sequencefloat.h"
#include "object/param/parameters.h"
#include "object/param/parameter.h"
#include "object/param/modulator.h"
#include "object/util/objectmodulatorgraph.h"
#include "object/util/objecteditor.h"
#include "object/util/audioobjectconnections.h"
#include "object/interface/valuetextureinterface.h"
#include "object/interface/valueshadersourceinterface.h"
#include "object/interface/geometryeditinterface.h"
#include "object/interface/evolutioneditinterface.h"
#include "gl/texture.h"
#include "model/objectmimedata.h"
#include "model/objecttreemimedata.h"
#include "tool/actionlist.h"
#include "io/files.h"
#include "io/currenttime.h"
#include "io/application.h"
#include "io/log_gui.h"

#include "object/util/parameterevolution.h"

namespace MO {
namespace GUI {

namespace
{
    QRectF bounding_rect(const QPointF& one, const QPointF& two)
    {
        QRectF r;
        r.setLeft(  std::min(one.x(), two.x()));
        r.setRight( std::max(one.x(), two.x()));
        r.setTop(   std::min(one.y(), two.y()));
        r.setBottom(std::max(one.y(), two.y()));
        return r;
    }
}

class ObjectGraphScene::Private
{
public:
    enum Action
    {
        A_NONE,
        A_DRAG_SPACE,
        A_RECT_SELECT,
        A_DRAG_CONNECT
    };

    Private(ObjectGraphScene * scene)
        :   scene   (scene),
            editor  (0),
            zStack  (0),
            action  (A_NONE),
            connectStartConnectItem (0),
            connectStartItem    (0),
            connectEndItem      (0),
            nextSelectedObject  (0)
    { }

    /// Creates the items for o and all of its children
    void createObjectItem(Object * o, const QPoint &local_pos);
    /// Creates the items for all children of o and recursively
    void createObjectChildItems(Object * o, AbstractObjectItem * item);
    void createModulatorItems(Object * root);
    /// Completely recreates all cables (to nasty to do it selectively)
    void recreateModulatorItems();
    void addAudioCon(AudioObjectConnection*);
    void addModItem(Modulator *);
    void addModItemMap(Object *, ModulatorItem *);
    void addConItemMap(Object *, AudioConnectionItem *);
    /// Recursively get the item below @p localGridPos
    AbstractObjectItem * childItemAt(AbstractObjectItem * parent, const QPoint& localGridPos);
    ObjectGraphConnectItem * connectorAt(const QPointF& scenePos);
    /// Raises all ModulatorItems of the item and it's childs
    void raiseModItems(AbstractObjectItem*);
    /** Resizes the items to fit their children and updates cable positions.
        Also updates the connectors on each module */
    void resolveLayout(bool updateConnections = false);

    void clearActions();
    void showPopup(); ///< Runs popup after actions have been created
    void createNewObjectMenu(ActionList& actions, Object * o);
    void createClipboardMenu(ActionList& actions, Object * parent, const QList<AbstractObjectItem*>& items);
    void createObjectEditMenu(ActionList& actions, Object * o);
    void createSaveTextureMenu(ActionList& actions, Object * obj);
    //void createObjectGroupMenu(ActionList& actions, const QList<AbstractObjectItem*>& items);
    QMenu * createObjectsMenu(Object *parent, bool with_template, bool with_shortcuts,
                              bool childObjects = true, int groups = Object::TG_ALL);

    void snapToEndConnect(const QPointF& scenePos);
    void endConnection();

    void acceptDrag(QGraphicsSceneDragDropEvent*);

    void saveTexture(const GL::Texture*);

    ObjectGraphScene * scene;
    Scene * root;
    ObjectEditor * editor;
    std::map<Object*, AbstractObjectItem*> itemMap;
    std::map<Object*, QList<ModulatorItem*>> modItemMap;
    std::map<Object*, QList<AudioConnectionItem*>> conItemMap;
    QList<ModulatorItem*> modItems;
    QList<AudioConnectionItem*> audioConItems;
    int zStack; ///< highest current stack value
    Action action;
    QPointF lastMousePos;
    QRectF selUpdateRect;

    ActionList actions;
    QPoint popupGridPos;
    ObjectGraphConnectItem * connectStartConnectItem, * connectEndConnectItem;
    AbstractObjectItem * connectStartItem, * connectEndItem;
    QPointF connectStartPos, connectEndPos;

    Object * nextSelectedObject;
};


ObjectGraphScene::ObjectGraphScene(QObject *parent)
    : QGraphicsScene    (parent),
      p_                (new Private(this))
{
    // capture changes to graphicsitems
    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(onChanged_()));
}

ObjectGraphScene::~ObjectGraphScene()
{
    delete p_;
}

AbstractObjectItem * ObjectGraphScene::itemForObject(const Object *o) const
{
    auto i = p_->itemMap.find(const_cast<Object*>(o));
    return i == p_->itemMap.end() ? 0 : i->second;
}

AbstractObjectItem * ObjectGraphScene::visibleItemForObject(Object *o) const
{
    auto i = p_->itemMap.find(o);
    if (i == p_->itemMap.end())
        return 0;

    if (i->second->isVisible())
        return i->second;

    auto p = i->second->parentObjectItem();
    if (p->isVisible())
        return p;

    while (p && !p->isVisible())
        p = p->parentObjectItem();

    return p;
}

void ObjectGraphScene::setRootObject(Object *root)
{
    clear();

    p_->itemMap.clear();
    p_->modItemMap.clear();
    p_->conItemMap.clear();
    p_->modItems.clear();
    p_->audioConItems.clear();
    p_->zStack = 0;
    p_->root = dynamic_cast<Scene*>(root);
    p_->action = Private::A_NONE;

    if (p_->root)
    {
        MO_ASSERT(p_->root->editor(), "ObjectGraphScene given Scene without ObjectEditor");

        // install editor
        bool changed = (p_->root->editor() != p_->editor);
        p_->editor = p_->root->editor();
        if (changed)
        {
            connect(p_->editor, SIGNAL(objectAdded(MO::Object*)),
                    this, SLOT(onObjectAdded_(MO::Object*)));
            connect(p_->editor, SIGNAL(objectsAdded(QList<MO::Object*>)),
                    this, SLOT(onObjectsAdded_(QList<MO::Object*>)));
            connect(p_->editor, SIGNAL(objectDeleted(const MO::Object*)),
                    this, SLOT(onObjectDeleted_(const MO::Object*)));
            connect(p_->editor, SIGNAL(objectsDeleted(QList<MO::Object*>)),
                    this, SLOT(onObjectsDeleted_(QList<MO::Object*>)));
            connect(p_->editor, SIGNAL(objectMoved(MO::Object*,MO::Object*)),
                    this, SLOT(onObjectMoved_(MO::Object*,MO::Object*)));
            connect(p_->editor, SIGNAL(objectChanged(MO::Object*)),
                    this, SLOT(onObjectChanged_(MO::Object*)));
            connect(p_->editor, SIGNAL(objectNameChanged(MO::Object*)),
                    this, SLOT(onObjectNameChanged_(MO::Object*)));
            connect(p_->editor, SIGNAL(objectColorChanged(MO::Object*)),
                    this, SLOT(onObjectColorChanged_(MO::Object*)));
            connect(p_->editor, SIGNAL(modulatorAdded(MO::Modulator*)),
                    this, SLOT(onModulatorAdded_(MO::Modulator*)));
            connect(p_->editor, SIGNAL(modulatorDeleted(const MO::Modulator*)),
                    this, SLOT(onModulatorDeleted_()));
            connect(p_->editor, SIGNAL(modulatorsDeleted(QList<MO::Modulator*>)),
                    this, SLOT(onModulatorDeleted_()));
            connect(p_->editor, SIGNAL(audioConnectionsChanged()),
                    this, SLOT(onConnectionsChanged_()));
            connect(p_->editor, SIGNAL(parameterVisibilityChanged(MO::Parameter*)),
                    this, SLOT(onParameterVisibilityChanged_(MO::Parameter*)));
            connect(p_->editor, SIGNAL(parameterChanged(MO::Parameter*)),
                    this, SLOT(onParameterChanged_(MO::Parameter*)));
        }
    }

    p_->createObjectChildItems(root, 0);
    p_->resolveLayout();
    p_->createModulatorItems(root);

    //for (auto & p : p_->itemMap)
    //    MO_DEBUG(p.first->name() << " :: " << p.second);
}



QPoint ObjectGraphScene::mapToGrid(const QPointF & f) const
{
    const auto s = ObjectGraphSettings::gridSize();
    const int ox = f.x() < 0 ? -1 : 0,
              oy = f.y() < 0 ? -1 : 0;
    return QPoint(int(f.x() / s.width()) + ox,
                  int(f.y() / s.height()) + oy);
}

QPointF ObjectGraphScene::mapFromGrid(const QPoint & p) const
{
    const auto s = ObjectGraphSettings::gridSize();
    return QPointF(p.x() * s.width(),
                   p.y() * s.height());
}



QList<AbstractObjectItem*> ObjectGraphScene::selectedObjectItems() const
{
    auto list = selectedItems();

    QList<AbstractObjectItem*> l2;
    for (auto i : list)
        if (i->type() >= AbstractObjectItem::T_BASE)
            l2 << static_cast<AbstractObjectItem*>(i);

    return l2;
}


/** @todo this happens quite too often, even for button-less mouse-moves!! */
void ObjectGraphScene::onChanged_()
{
    //MO_DEBUG("changed");
    p_->resolveLayout();
}

void ObjectGraphScene::Private::createObjectChildItems(Object *o, AbstractObjectItem * pitem)
{
    if (!o->isVisible())
        return;

    int y = 1;
    for (Object * c : o->childObjects())
    if (c->isVisible())
    {
        // create item
        auto item = new AbstractObjectItem(c);
        // save in map
        itemMap.insert(std::make_pair(c, item));

        //o->dumpAttachedData();

        // set expanded according to gui settings
        //if (c->hasAttachedData(Object::DT_GRAPH_EXPANDED))
        //    item->setExpanded(c->getAttachedData(Object::DT_GRAPH_EXPANDED).toBool());

        // set initial position (from gui settings or top-to-bottom)
        QPoint pos;
        if (c->hasAttachedData(Object::DT_GRAPH_POS))
            pos = c->getAttachedData(Object::DT_GRAPH_POS).toPoint();
        else
            pos = QPoint(1, y);

        // limit to child area and find free pos
        if (pitem)
            pos = scene->nextFreePosition(pitem, pos);

        item->setGridPos(pos);
        // remember y extent
        y = std::max(y, item->gridRect().bottom() + 1);

        // install in item tree
        if (!pitem)
            scene->addItem(item);
        else
            item->setParentItem(pitem);
        item->setZValue(++zStack);

        // add childs
        createObjectChildItems(c, item);
    }
}

void ObjectGraphScene::Private::createModulatorItems(Object *root)
{
    /*ObjectGraph graph;
    getObjectModulatorGraph(graph, root);

    for (auto & i : graph)
    {
        ObjectGraphNode * n = i.second;
    }*/

    // add modulators
    auto mods = root->getModulators();
    for (Modulator * m : mods)
    {
        if (m->modulator())
            addModItem(m);
        else
            MO_WARNING("unassigned modulator for item, id == " << m->modulatorId());
    }

    // add audio connections
    if (auto ao = dynamic_cast<AudioObject*>(root))
    {
        auto list = this->root->audioConnections()->getInputs(ao);
        for (auto c : list)
            addAudioCon(c);
    }

    // process childs
    for (auto c : root->childObjects())
        createModulatorItems(c);
}

void ObjectGraphScene::Private::addModItem(Modulator * m)
{
    // find parent for connection;
    Object * parent = m->modulator()->findCommonParentObject(m->parent());

    QGraphicsItem * pitem = scene->itemForObject(parent);

    auto item = new ModulatorItem(m, pitem);
    if (!pitem)
        scene->addItem( item );

    item->setZValue(++zStack);
    item->updateShape(); // calc arrow shape

    // store in map
    modItems.append( item );
    // ModulatorItems per object
    addModItemMap(m->parent(), item);
    addModItemMap(m->modulator(), item);
}

void ObjectGraphScene::Private::addAudioCon(AudioObjectConnection * c)
{
    auto item = new AudioConnectionItem(c);
    scene->addItem( item );
    item->setZValue(++zStack);
    item->updateShape(); // calc arrow shape

    // store in map
    audioConItems.append( item );
    // ModulatorItems per object
    addConItemMap(c->from(), item);
    addConItemMap(c->to(), item);
}

void ObjectGraphScene::Private::addModItemMap(Object * o, ModulatorItem * item)
{
    auto i = modItemMap.find(o);
    if (i == modItemMap.end())
    {
        modItemMap.insert(std::make_pair(o, QList<ModulatorItem*>() << item));
    }
    else
    {
        i->second << item;
    }
}

void ObjectGraphScene::Private::addConItemMap(Object * o, AudioConnectionItem * item)
{
    auto i = conItemMap.find(o);
    if (i == conItemMap.end())
    {
        conItemMap.insert(std::make_pair(o, QList<AudioConnectionItem*>() << item));
    }
    else
    {
        i->second << item;
    }
}

void ObjectGraphScene::Private::createObjectItem(Object *o, const QPoint& local_pos)
{
    MO_DEBUG_GUI("ObjectGraphScene::createObjectItem(" << o << ", QPoint("
             << local_pos.x() << ", " << local_pos.y() << "))");

    // create item
    auto item = new AbstractObjectItem(o);
    // save in map
    itemMap.insert(std::make_pair(o, item));

    // item for parent
    auto pitem = scene->itemForObject(o->parentObject());

    // set initial position
    QPoint pos = o->parentObject()
            ? scene->nextFreePosition(o->parentObject(), local_pos)
            : local_pos;

    // limit to parent rect
    if (o->parentObject() && o->parentObject() != o->sceneObject())
    {
        if (pos.x() < 1) pos.setX(1);
        if (pos.y() < 1) pos.setY(1);
    }

    item->setGridPos(pos);
    item->setZValue(++zStack);

    const bool isExpand = (o->isAudioObject() && !o->hasAttachedData(Object::DT_GRAPH_EXPANDED))
                        || o->getAttachedData(Object::DT_GRAPH_EXPANDED).toBool();
    item->setExpanded(isExpand);

    // install in item tree
    if (!pitem)
        scene->addItem(item);
    else
    {
        item->setParentItem(pitem);
        pitem->setExpanded();
    }

    // add childs
    createObjectChildItems(o, item);

    // create modulator items
    createModulatorItems(o);

    MO_DEBUG(nextSelectedObject << " " << o);
    if (nextSelectedObject == o)
    {
        nextSelectedObject = 0;
        item->setSelected(true);
    }
}

void ObjectGraphScene::Private::resolveLayout(bool updateConnections)
{
    auto list = scene->items();

    bool change = false;

    // set all item grid sizes according to their childs
    for (QGraphicsItem * item : list)
    if (item->type() >= AbstractObjectItem::T_BASE)
    {
        auto o = static_cast<AbstractObjectItem *>(item);

        if (updateConnections)
            o->updateConnectors();

        // get dirty root items
        if (!o->isLayouted() && !o->parentItem())
        {
            o->adjustSizeToChildren();
            o->setLayoutDirty(false);
            change = true;
        }
    }

    list = scene->items();

    // update modulator items
    if (change)
    {
        for (QGraphicsItem * item : list)
        {
            if (item->type() == ModulatorItem::Type)
            {
                auto m = static_cast<ModulatorItem*>(item);
                m->updateShape();
            }

            if (item->type() == AudioConnectionItem::Type)
            {
                auto m = static_cast<AudioConnectionItem*>(item);
                m->updateShape();
            }
        }
    }
}

void ObjectGraphScene::repaintModulators(AbstractObjectItem * )
{
    // XXX currently updates all !! could be more efficent
    for (ModulatorItem * i : p_->modItems)
    {
        i->update();
    }
    /*
    auto i = p_->modItemMap.find(item->object());
    while (i != p_->modItemMap.end())
    {
        i->second->update();
        ++i;
    }*/
}

void ObjectGraphScene::toFront(QGraphicsItem * item)
{
    item->setZValue(++p_->zStack);

    // raise ModulatorItems
    if (item->type() >= AbstractObjectItem::T_BASE)
    {
        auto oi = static_cast<AbstractObjectItem*>(item);
        p_->raiseModItems(oi);
    }
}

void ObjectGraphScene::Private::raiseModItems(AbstractObjectItem * item)
{
    // raise all connected ModulatorItems
    {
        auto i = modItemMap.find(item->object());
        if (i != modItemMap.end())
        {
            for (auto mi : i->second)
                mi->setZValue(++zStack);
        }
    }

    // raise all connected AudioConnectionItems
    auto i = conItemMap.find(item->object());
    if (i != conItemMap.end())
    {
        for (auto mi : i->second)
            mi->setZValue(++zStack);
    }

    // traverse AbstractObjectItem childs
    const auto list = item->childItems();
    for (auto c : list)
        if (c->type() >= AbstractObjectItem::T_BASE)
            raiseModItems(static_cast<AbstractObjectItem*>(c));
}



void ObjectGraphScene::Private::recreateModulatorItems()
{
    for (auto m : modItems)
    {
        scene->removeItem(m);
        delete m;
    }
    for (auto m : audioConItems)
    {
        scene->removeItem(m);
        delete m;
    }
    modItems.clear();
    modItemMap.clear();
    audioConItems.clear();
    conItemMap.clear();

    resolveLayout();

    if (root)
        createModulatorItems(root);
}

QList<AbstractObjectItem*> ObjectGraphScene::topLevelObjectItems() const
{
    QList<AbstractObjectItem*> ret;

    // XXX This sometimes crashes during left-click popup
    // because of an invalid item in p_->itemMap
//    for (auto & i : p_->itemMap)
//    if (!i.second->parentItem())
//        ret << i.second;
    auto list = items();
    for (auto i : list)
        if (i->type() >= AbstractObjectItem::T_BASE
                && !i->parentItem())
            ret << static_cast<AbstractObjectItem*>(i);

    return ret;
}

void ObjectGraphScene::reduceToTopLevel(QList<AbstractObjectItem *> & input)
{
    // get objects
    auto set = input.toSet();

    // filter only top-level objects
    QList<AbstractObjectItem*> toplevel;
    for (auto i : set)
    {
        auto tl = i;
        while ((i = i->parentObjectItem()))
        {
            if (set.contains(i))
            {
                tl = 0;
                break;
            }
        }

        if (tl)
            toplevel << tl;
    }

    if (toplevel.size() != input.size())
        std::swap(input, toplevel);
}

AbstractObjectItem * ObjectGraphScene::objectItemAt(const QPoint &gridPos)
{
    // NOTE: Lets not rely on QGraphicsScene::itemAt because
    // cables could come in the way

    if (!p_->root)
        return 0;

    const auto list = topLevelObjectItems();
    for (AbstractObjectItem * i : list)
        if (auto ret = p_->childItemAt(i, gridPos - i->gridPos()))
            return ret;

    return 0;
}

AbstractObjectItem * ObjectGraphScene::objectItemAt(
        AbstractObjectItem * parent, const QPoint &pos)
{
    // NOTE: Lets not rely on QGraphicsScene::itemAt because
    // cables could come in the way

    if (!parent)
        return objectItemAt(pos);

    if (!p_->root)
        return 0;

    const auto list = parent->childItems();
    for (QGraphicsItem * gi : list)
    if (gi->type() >= AbstractObjectItem::T_BASE)
    {
        auto oi = static_cast<AbstractObjectItem*>(gi);
        // check if pos is inside child
        auto r = oi->gridRect();
        if (pos.x() < r.x() || pos.y() < r.y()
            || pos.x() > r.right() || pos.y() > r.bottom())
            continue;
        // check for sub-child
        if (auto ret = objectItemAt(oi, pos - r.topLeft()))
            return ret;
        // return this child
        return oi;
    }
    return 0;
}

QPoint ObjectGraphScene::nextFreePosition(Object *parent, const QPoint &pos1) const
{
    auto item = itemForObject(parent);
    if (!item)
        /** @todo find next free position in scene! */
        return pos1;
    return nextFreePosition(item, pos1);
}

QPoint ObjectGraphScene::nextFreePosition(AbstractObjectItem * item, const QPoint &pos1) const
{
    QPoint pos(pos1);
    // limit to child area
    pos.setX( std::max(pos.x(), 1));
    pos.setY( std::max(pos.y(), 1));
    //MO_DEBUG("find next free pos for " << pos.x() << ", " << pos.y());
    auto i = p_->childItemAt(item, pos);
    while (i != 0 && i != item)
    {
        //qDebug() << "failed for" << pos;
        ++pos.ry();
        i = p_->childItemAt(item, pos);
    }
    //MO_DEBUG("return pos == " << pos.x() << ", " << pos.y());
    return pos;
}

AbstractObjectItem * ObjectGraphScene::Private::childItemAt(AbstractObjectItem * parent, const QPoint& pos)
{
    // pos is local in parent

    if (pos.x() < 0 || pos.y() < 0
        || pos.x() >= parent->gridSize().width()
        || pos.y() >= parent->gridSize().height())
        return 0;

    const auto list = parent->childItems();
    for (auto i : list)
    if (i->type() >= AbstractObjectItem::T_BASE)
    {
        auto o = static_cast<AbstractObjectItem*>(i);

        if (auto ret = childItemAt(o, pos - o->gridPos()))
            return ret;
    }

    return parent;
}

void ObjectGraphScene::setFocusObject(Object *o)
{
    auto item = itemForObject(o);
    if (!item)
        return;

    // find the parent if not expanded
    while (item && !item->isVisible())
        item = item->parentObjectItem();

    if (!item)
        return;

    clearSelection();
    item->setSelected(true);
    item->ensureVisible();
}


bool ObjectGraphScene::startConnection(ObjectGraphConnectItem * item)
{
    auto actual = itemForObject(item->object());
    if (!actual)
        return false;

    p_->connectStartConnectItem = item;
    p_->connectStartItem = actual;
    p_->connectEndItem = 0;
    p_->connectEndConnectItem = 0;

    p_->connectStartPos = p_->connectEndPos = item->scenePos();
    p_->action = Private::A_DRAG_CONNECT;

    update();
    return true;
}

ObjectGraphConnectItem * ObjectGraphScene::Private::connectorAt(const QPointF &scenePos)
{
    // XXX refine this (other items are in the way)
    QGraphicsItem * some = scene->itemAt(scenePos, QTransform());

    return qgraphicsitem_cast<ObjectGraphConnectItem*>(some);
}

void ObjectGraphScene::Private::snapToEndConnect(const QPointF& scenePos)
{
    connectEndPos = scenePos;

    // find goal item
    connectEndItem = scene->objectItemAt(scene->mapToGrid(connectEndPos));
    if (connectEndItem == connectStartItem)
        connectEndItem = 0;

    if (!connectEndItem)
    {
        connectEndConnectItem = 0;
        return;
    }

    connectEndConnectItem = connectorAt(scenePos);

    if (connectEndConnectItem)
        connectEndPos = connectEndConnectItem->mapToScene(0,0);
/*
    // snap to input of audioobject
    if (connectEndItem->object()->isAudioObject())
    {
        int chan = connectEndItem->channelForPosition(
                    connectEndItem->mapFromScene(connectEndPos));
        connectEndPos = connectEndItem->globalInputPos(chan >= 0 ? chan : 0);
    }
    else
    {
        // snap to parameter connector
        if (auto con = connectorAt(scenePos))
        {
            connectEndPos = con->mapToScene(0,0);
            connectEndItem = con->objectItem();
        }
        else
            connectEndPos = connectEndItem->globalInputPos(0);
    }
*/
}

void ObjectGraphScene::Private::endConnection()
{
    if (connectStartConnectItem->signalType() == ST_AUDIO)
    {
        auto aoFrom = dynamic_cast<AudioObject*>(connectStartItem->object()),
             aoTo = dynamic_cast<AudioObject*>(connectEndItem->object());

        if (aoFrom && aoTo)
        {
            // find channel of connector
            int chan = connectEndItem->channelForPosition(
                        connectEndItem->mapFromScene(connectEndPos));
            if (chan >= 0)
                editor->connectAudioObjects(
                        aoFrom, aoTo,
                        connectStartConnectItem->channel(), chan,
                        1);
        }
        else
        // create audio to parameter modulation
        if (connectEndConnectItem && connectEndConnectItem->parameter())
        {
            editor->addModulator(
                        connectEndConnectItem->parameter(),
                        aoFrom->idName(),
                        QString("_audio_%1").arg(connectStartConnectItem->channel()));
        }
        else
        {
            MO_WARNING("aoFrom/aoTo not found " << aoFrom << "/" << aoTo
                       << ". Could not establish audio connection");
        }
    }
    // any type
    else
    {
        auto from = connectStartItem->object(),
                to = connectEndItem->object();
        if (from && to && connectEndConnectItem)
        {
            if (connectEndConnectItem->isParameter())
            {
                editor->addModulator(
                            connectEndConnectItem->parameter(),
                            from->idName(),
                            QString::number(connectStartConnectItem->channel()));
            }
        }
    }

    // reset those
    connectStartConnectItem = 0;
    connectEndConnectItem = 0;
    connectStartItem = 0;
    connectEndItem = 0;
}




// ----------------------------- drag/drop --------------------------------------

void ObjectGraphScene::Private::acceptDrag(QGraphicsSceneDragDropEvent * e)
{
//    qInfo() << "accept?" << e->mimeData()->formats();

    if (!root)
        return;

    // filename(s)
    if (e->mimeData()->hasUrls())
    {
        e->accept();
        return;
    }

    if (e->mimeData()->hasFormat(ObjectMimeData::mimeTypeString))
    {
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
        if (typ < 0 || !root->canHaveChildren(Object::Type(typ)))
            return;

        e->accept();
        return;
    }
}

void ObjectGraphScene::dragEnterEvent(QGraphicsSceneDragDropEvent * e)
{
    // check items
    QGraphicsScene::dragEnterEvent(e);
    // restore action from Qt::IgnoreAction
    e->setDropAction(e->proposedAction());

    // check self
    if (!e->isAccepted())
        p_->acceptDrag(e);
}

// For some reason, QGraphicsScene must accept the event in dragMOVE as well,
// otherwise it's rejected..
void ObjectGraphScene::dragMoveEvent(QGraphicsSceneDragDropEvent * e)
{
    // check items
    QGraphicsScene::dragMoveEvent(e);
    // restore action from Qt::IgnoreAction
    e->setDropAction(e->proposedAction());

    // check self
    if (!e->isAccepted())
        p_->acceptDrag(e);
}


void ObjectGraphScene::dropEvent(QGraphicsSceneDragDropEvent * e)
{
    // pass drop event to items first
    e->ignore();
    QGraphicsScene::dropEvent(e);
    //qInfo() << "after org: accepted ==" << e->isAccepted();

    // otherwise drop into root (scene)
    if (!e->isAccepted())
        dropAction(e);
}



void ObjectGraphScene::dropAction(QGraphicsSceneDragDropEvent *e, Object * parent)
{
    QPointF ePos = e->pos();
    if (!parent)
    {
        parent = p_->root;
        ePos = e->scenePos();
    }
    if (!parent)
        return;

    // filename(s)
    if (e->mimeData()->hasUrls())
    {
        e->accept();
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
            addObjects(parent, objs, mapToGrid(ePos));
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

        if (desc.pointer())
            popupObjectDrag(desc.pointer(), parent, e->scenePos());

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
        if (typ < 0 || !parent->canHaveChildren(Object::Type(typ)))
            return;

        addObject(parent,
                  ObjectFactory::createObject(classn),
                  mapToGrid(ePos)
                  );

        e->accept();
        return;
    }
}




// ------------------------------ mouse events ----------------------------------

void ObjectGraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
/*    auto it = objectItemAt(mapToGrid(event->scenePos()));
    if (it)
        MO_DEBUG(it->object()->name());
*/
    // finish connection
    if (p_->action == Private::A_DRAG_CONNECT)
    {
        if (event->button() == Qt::LeftButton
            && p_->connectEndItem && p_->connectStartItem)
            p_->endConnection();
        p_->action = Private::A_NONE;
        //update(sceneRect());
        update(bounding_rect(p_->connectStartPos, p_->connectEndPos).adjusted(-100,-100,100,100));

        event->accept();
        return;
    }

    // process items
    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted())
    {
        createEditMenu();
        return;
    }

    if (event->button() == Qt::RightButton)
    {
        p_->lastMousePos = event->scenePos();
        popup(mapToGrid(p_->lastMousePos));
    }

    if (event->button() == Qt::LeftButton)
    {
        // start drag-select
        p_->action = Private::A_RECT_SELECT;
        p_->lastMousePos = event->scenePos();
        setSelectionArea(QPainterPath());
    }

    createEditMenu();
}

void ObjectGraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // draw a connection
    if (p_->action == Private::A_DRAG_CONNECT)
    {
        update(bounding_rect(p_->connectStartPos, p_->connectEndPos).adjusted(-100,-100,100,100));

        p_->snapToEndConnect(event->scenePos());

        update(bounding_rect(p_->connectStartPos, p_->connectEndPos).adjusted(-100,-100,100,100));
        return;
    }

    // process childs
    QGraphicsScene::mouseMoveEvent(event);
    if (event->isAccepted())
        return;

    if (p_->action == Private::A_DRAG_SPACE)
    {
        emit shiftView(event->pos() - p_->lastMousePos);
    }

    if (p_->action == Private::A_RECT_SELECT)
    {
        // update previous sel-area
        update(p_->selUpdateRect);
        // make new
        p_->selUpdateRect = bounding_rect(  p_->lastMousePos, event->scenePos() );
        QPainterPath p;
        p.addRect(p_->selUpdateRect);
        setSelectionArea(p);
        p_->selUpdateRect.adjust(-1,-1,2,2);
        update(p_->selUpdateRect);
    }
}

void ObjectGraphScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{

    QGraphicsScene::mouseReleaseEvent(event);
    if (event->isAccepted())
    {
        createEditMenu();
        return;
    }

    if (p_->action == Private::A_RECT_SELECT)
    {
        update(p_->selUpdateRect);
        //setSelectionArea(QPainterPath());
    }

    p_->action = Private::A_NONE;
    createEditMenu();
}





// ------------------------------------- drawing -------------------------------

void ObjectGraphScene::drawForeground(QPainter *p, const QRectF &)
{
    // selection frame
    if (p_->action == Private::A_RECT_SELECT)
    {
        p->setBrush(Qt::NoBrush);
        p->setPen(ObjectGraphSettings::penSelectionFrame());

        p->drawPath(selectionArea());
    }

    // audio connection dragging
    if (p_->action == Private::A_DRAG_CONNECT)
    {
        // check current goal
        Object * goal = 0;
        if (p_->connectEndItem)
            goal = p_->connectEndItem->object();

        // set pen
        QPen pen = QPen(goal
                        ? ObjectFactory::colorForObject(goal)
                        : QColor(255, 125, 125));
        pen.setWidth(p_->connectEndConnectItem ? 2.7 : 1.0);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);

        p->drawPath(ObjectGraphSettings::pathWire(
                        p_->connectStartPos,
                        p_->connectEndPos));
    }
}


// --------------------------------- menu --------------------------------------

void ObjectGraphScene::Private::showPopup()
{
    auto popup = new QMenu(application()->mainWindow());
    popup->setAttribute(Qt::WA_DeleteOnClose, true);

    popup->addActions(actions);

    // don't dispay if empty (first two actions are title & separator)
    if (actions.size() <= 1
        || (actions.size() <= 2 && actions[1]->isSeparator()))
        popup->close();
    else
        popup->popup(QCursor::pos());
}

void ObjectGraphScene::Private::clearActions()
{
    for (QAction * a : actions)
    {
        if (a->menu() && !a->menu()->parent())
            delete a->menu();
        a->deleteLater();
    }
    actions.clear();
}

void ObjectGraphScene::createEditMenu()
{
    auto items = selectedObjectItems();

    Object * obj = items.isEmpty() ? p_->root : items.first()->object();

    if (!obj)
        return;

    ActionList actions;

    // title
    QString title(items.size() < 2
                  ? obj->name()
                  : tr("%1 objects").arg(items.size()));
    actions.addTitle(title, this);

    // edit object
    if (items.size() == 1)
        p_->createObjectEditMenu(actions, obj);

    if (items.size() < 2)
    {
        // new object
        p_->createNewObjectMenu(actions, obj);
    }

    // clipboard
    p_->createClipboardMenu(actions, obj, items);

    emit editActionsChanged(actions);
}

void ObjectGraphScene::popup(const QPoint& gridPos)
{
    p_->clearActions();

    auto items = selectedObjectItems();

    Object * obj = items.isEmpty() ? p_->root : items.first()->object();

    if (!obj)
        return;

    p_->popupGridPos = gridPos;

    // title
    QString title(items.size() < 2
                  ? obj->name()
                  : tr("%1 objects").arg(items.size()));
    p_->actions.addTitle(title, this);

    // edit object
    if (items.size() == 1)
        p_->createObjectEditMenu(p_->actions, obj);

    if (items.size() < 2)
    {
        // new object
        p_->createNewObjectMenu(p_->actions, obj);
    }

    // clipboard
    p_->createClipboardMenu(p_->actions, obj, items);

    p_->showPopup();
}

void ObjectGraphScene::popup(AudioObjectConnection * con)
{
    p_->clearActions();

    // title
    QString title(tr("audio connection"));
    p_->actions.addTitle(title, this);

    // delete
    QAction * a = p_->actions.addAction(tr("remove connection"), this);
    a->setStatusTip(tr("Removes the selected connection"));
    connect(a, &QAction::triggered, [this, con]()
    {
        p_->editor->disconnectAudioObjects(*con);
    });


    p_->showPopup();
}

void ObjectGraphScene::popup(Modulator * mod)
{
    p_->clearActions();

    // title
    QString title(mod->nameAutomatic());
    p_->actions.addTitle(title, this);

    // edit
    QAction * a = p_->actions.addAction(tr("edit"), this);
    a->setStatusTip(tr("Opens a dialog to edit the properties of the modulation"));
    connect(a, &QAction::triggered, [this, mod]()
    {
        ModulatorDialog diag;
        diag.setModulators(QList<Modulator*>() << mod, mod);
        diag.move(QCursor::pos());
        diag.exec();
    });

    p_->actions.addSeparator(this);

    // delete
    a = p_->actions.addAction(tr("remove modulation"), this);
    a->setStatusTip(tr("Removes the selected modulation, eveything goes back to normal"));
    connect(a, &QAction::triggered, [this, mod]()
    {
        p_->editor->removeModulator(mod->parameter(), mod->modulatorId(), mod->outputId());
    });


    p_->showPopup();
}


void ObjectGraphScene::popupObjectDrag(Object * source, Object * goal, const QPointF& dropPointF)
{
    if (!p_->editor || source == goal)
        return;

    p_->clearActions();

    // title
    QString title(source->name() + " > " + goal->name());
    p_->actions.addTitle(title, this);

    p_->actions.addSeparator(this);

    // --- move here ---

    if (goal != source->parentObject() && !goal->hasParentObject(source))
    {
        QAction * a = p_->actions.addAction(tr("move into %1").arg(goal->name()), this);
        connect(a, &QAction::triggered, [=]()
        {
            // set destination grid position
            auto item = itemForObject(goal);
            QPoint gPos = item
                    ? mapToGrid(item->mapFromScene(dropPointF))
                    : mapToGrid(dropPointF);
            source->setAttachedData(gPos, Object::DT_GRAPH_POS);

            p_->editor->moveObject(source, goal);
        });

        p_->actions.addSeparator(this);
    }

    // --- modulator connect menu ---

    QMenu * menu = ObjectMenu::createParameterMenu(goal,0,
                        [=](Parameter * p)
                        {
                            return (p->getModulatorTypes() & source->type())
                                && !p->findModulator(source->idName());
                        });
    if (menu)
    {
        menu->setTitle(tr("connect to %1").arg(goal->name()));

        p_->actions.addMenu(menu, this);

        connect(menu, &QMenu::triggered, [=](QAction* a)
        {
            const QString id = a->data().toString();
            auto param = goal->params()->findParameter(id);
            if (param)
                p_->editor->addModulator(param, source->idName(), "");
        });
    }

    p_->showPopup();
}



void ObjectGraphScene::Private::createNewObjectMenu(ActionList &actions, Object * obj)
{
    actions.addSeparator(scene);

    if (!root)
    {
        MO_WARNING("Can't edit");
        return;
    }

    if (!obj)
        obj = root;

    QPoint objPos = QPoint(0, 0);
    if (obj != root)
        if (auto item = scene->itemForObject(obj))
            objPos = item->globalGridPos();

    QAction * a;

    // create new child
    actions.append( a = new QAction(obj && obj != root ?
                tr("new child object") : tr("new object"), scene) );

    QMenu * menu = createObjectsMenu(obj, true, false);
    a->setMenu(menu);
    connect(menu, &QMenu::triggered, [=](QAction*act)
    {
        QString id = act->data().toString();
        Object * onew;
        if (id == "_template_")
            onew = ObjectFactory::loadObjectTemplate();
        else
            onew = ObjectFactory::createObject(id);
        if (onew)
            scene->addObject(obj, onew, popupGridPos - objPos);
    });

    // append texture processor
    if (dynamic_cast<ValueTextureInterface*>(obj))
    {
        menu = createObjectsMenu(obj, true, false, false, Object::T_SHADER | Object::T_TEXTURE);
        if (menu)
        {
            actions.append( a = new QAction(tr("append texture processor"), scene) );
            a->setMenu(menu);

            connect(menu, &QMenu::triggered, [=](QAction*act)
            {
                QString id = act->data().toString();
                Object * onew = ObjectFactory::createObject(id);
                if (onew)
                    editor->appendTextureProcessor(obj, onew);
            });
        }
    }

}

void ObjectGraphScene::Private::createClipboardMenu(
        ActionList& actions, Object * /*parent*/, const QList<AbstractObjectItem*>& items)
{
    actions.addSeparator(scene);

    const bool plural = items.size() > 1;
    Object * obj = items.isEmpty() ? 0 : items.first()->object();

    QAction * a;

    if (obj && obj != root)
    {
        // copy
        a = actions.addAction(plural ? tr("Copy objects") : tr("Copy"), scene);
        a->setStatusTip(tr("Copies the selected object(s) and all it's children to the clipboard"));
        a->setShortcut(Qt::CTRL + Qt::Key_C);
        connect(a, &QAction::triggered, [this, items]()
        {
            application()->clipboard()->setMimeData(scene->mimeData(items));
        });

        // delete
        a = actions.addAction(plural ? tr("Delete objects") : tr("Delete"), scene);
        a->setStatusTip(tr("Deletes the object(s) from the scene"));
        a->setShortcut(Qt::CTRL + Qt::Key_Delete);
        connect(a, &QAction::triggered, [this, items]()
        {
            if (items.size() == 1)
                scene->deleteObject(items.first()->object());
            else
                scene->deleteObjects(items);
        });

    }

    if (!plural)
    {
        const auto data = application()->clipboard()->mimeData();
        const bool isClipboard = data->formats().contains(ObjectTreeMimeData::objectMimeType);

        if (isClipboard)
        {
            const auto pitem = scene->objectItemAt(popupGridPos);
            const QString pname = pitem && pitem->object()
                    ? pitem->object()->name() : tr("Scene");

            // paste
            a = actions.addAction(tr("Paste into %1").arg(pname), scene);
            a->setShortcut(Qt::CTRL + Qt::Key_V);
            connect(a, &QAction::triggered, [=]()
            {
                scene->dropMimeData(application()->clipboard()->mimeData(), popupGridPos);
            });
        }
    }
}

void ObjectGraphScene::Private::saveTexture(const GL::Texture* tex)
{
    auto fn = IO::Files::getSaveFileName(IO::FT_TEXTURE);
    if (fn.isEmpty())
        return;
    try
    {
        tex->bind();
        tex->saveImageFile(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(0, tr("saving image failed"), e.what() );
    }
}

void ObjectGraphScene::Private::createSaveTextureMenu(ActionList& actions, Object * obj)
{
    ValueTextureInterface * iface = dynamic_cast<ValueTextureInterface*>(obj);
    if (!iface)
        return;

    // XXX hacky in many regards
    RenderTime rt(CurrentTime::time(), 1./60., MO_GUI_THREAD);

    struct Tex
    {
        const GL::Texture* tex;
        int index;
    };

    // collect texture outputs
    std::vector<Tex> tex;
    for (size_t i=0; ; ++i)
    {
        auto t = iface->valueTexture(i, rt);
        if (!t || !t->isAllocated()
            // XXX can't save cube textures atm.
            || t->isCube())
        {
            if (i < 4)
                continue;
            else
                break;
        }
        Tex x;
        x.tex = t;
        x.index = i;
        tex.push_back(x);
    }

    if (tex.empty())
        return;

    QAction * a;

    // single output
    if (tex.size() == 1)
    {
        a = actions.addAction(QIcon(":/icon/disk.png"), tr("Save texture '%1'")
                              .arg(obj->getOutputName(ST_TEXTURE, tex[0].index)), scene);
        a->setStatusTip(tr("Saves the texture output to a file"));
        connect(a, &QAction::triggered, [=]() { saveTexture(tex[0].tex); });
    }
    else
    {
        auto sub = new QMenu(tr("Save output texture"));
        sub->setIcon(QIcon(":/icon/disk.png"));
        a = actions.addMenu(sub, scene);
        for (size_t i=0; i<tex.size(); ++i)
        {
            a = sub->addAction(obj->getOutputName(ST_TEXTURE, tex[i].index));
            a->setStatusTip(tr("Saves the texture output to a file"));
            connect(a, &QAction::triggered, [=]() { saveTexture(tex[i].tex); });
        }
    }
}

void ObjectGraphScene::Private::createObjectEditMenu(ActionList &actions, Object * obj)
{
    actions.addSeparator(scene);

    QAction * a;

    // display error
    if (obj->hasError())
    {
        QString err = obj->errorString();
        if (err.size() > 100)
            err = "..." + err.right(100);
        a = actions.addAction(tr("Error: %1").arg(err), scene);
        // XXX make it visible as error!
        // a->setIcon();
    }

    // set color
    a = actions.addAction(tr("Change color"), scene);
    auto sub = ObjectMenu::createHueMenu();
    a->setMenu(sub);
    connect(sub, &QMenu::triggered, [=](QAction * a)
    {
        int hue = a->data().toInt();
        editor->setObjectHue(obj, hue);
    });

    // GeometryDialog
    if (GeometryEditInterface * geom = dynamic_cast<GeometryEditInterface*>(obj))
    {
        // edit geometry
        a = actions.addAction(QIcon(":/icon/obj_geometry.png"), tr("Edit geometry"), scene);
        a->setStatusTip(tr("Opens a dialog for editing the attached geometry data"));
        connect(a, &QAction::triggered, [=]()
        {
            GeometryDialog::openForInterface(geom);
        });
    }

    // Model3d specific
    if (Model3d * model = dynamic_cast<Model3d*>(obj))
    {
        // show source code
        auto sub = new QMenu(tr("Show shader source"));
        a = actions.addMenu(sub, scene);
        a->setIcon(QIcon(":/icon/obj_glsl.png"));
        a->setStatusTip(tr("Shows the full shader source code including user-code, "
                           "automatic defines and overrides, and include files"));

        a = sub->addAction(tr("vertex shader"));
        a->setData(0);
        a = sub->addAction(tr("fragment shader"));
        a->setData(1);

        connect(sub, &QMenu::triggered, [=](QAction * a)
        {
            QString src = a->data().toInt() == 0
                    ? model->shaderSource().vertexSource()
                    : model->shaderSource().fragmentSource();

            auto diag = new TextEditDialog(src, TT_GLSL, application()->mainWindow());
            diag->setAttribute(Qt::WA_DeleteOnClose, true);
            diag->show();
        });
    }

    // TextureObject specific
    if (ValueShaderSourceInterface* ssrc = dynamic_cast<ValueShaderSourceInterface*>(obj))
    {
        // show source code
        auto sub = new QMenu(tr("Show shader source"));
        a = actions.addMenu(sub, scene);
        a->setIcon(QIcon(":/icon/obj_glsl.png"));
        a->setStatusTip(tr("Shows the full shader source code including user-code, "
                           "automatic defines and overrides, and include files"));

        a = sub->addAction(tr("vertex shader"));
        a->setData(0);
        a = sub->addAction(tr("fragment shader"));
        a->setData(1);

        connect(sub, &QMenu::triggered, [=](QAction * a)
        {
            QString src = a->data().toInt() == 0
                    ? ssrc->valueShaderSource(0).vertexSource()
                    : ssrc->valueShaderSource(0).fragmentSource();

            auto diag = new TextEditDialog(src, TT_GLSL, application()->mainWindow());
            diag->setAttribute(Qt::WA_DeleteOnClose, true);
            diag->show();
        });

    }

    // wrap float sequence in float track
    if (SequenceFloat * seq = dynamic_cast<SequenceFloat*>(obj))
    if (!obj->findParentObject(Object::T_TRACK_FLOAT))
    {
        a = actions.addAction(tr("Wrap into track"), scene);
        a->setStatusTip(tr("Creates a track and puts the sequence inside"));
        connect(a, &QAction::triggered, [=]()
        {
            editor->wrapIntoTrack(seq);
        });
    }

    // EvolutionDialog
    if (EvolutionEditInterface * evo = dynamic_cast<EvolutionEditInterface*>(obj))
    {
        for (int i=0; i<evo->evolutionKeys().size(); ++i)
        {
            a = actions.addAction(tr("Evolve %1").arg(evo->evolutionKeys()[i]), scene);
            a->setStatusTip(tr("Opens the evolution dialog for editing the specimen"));
            connect(a, &QAction::triggered, [=]()
            {
                EvolutionDialog::openForInterface(evo, evo->evolutionKeys()[i]);
            });
        }
    }


    actions.addSeparator(scene);

    a = actions.addAction(QIcon(":/icon/disk.png"), tr("Save as template ..."), scene);
    a->setStatusTip(tr("Saves the object and it's sub-tree to a file for later reuse"));
    connect(a, &QAction::triggered, [=]()
    {
        ObjectFactory::storeObjectTemplate(obj);
    });

    // save texture output
    if (dynamic_cast<ValueTextureInterface*>(obj))
    {
        createSaveTextureMenu(actions, obj);
    }

}

/*
void ObjectGraphScene::Private::createObjectGroupMenu(
        ActionList& actions, const QList<AbstractObjectItem*>& items)
{
    if (!items.size() > 1)
        return;

    QAction * a;
    actions.addSeparator(scene);

    //Object * obj = items.first()->object();
    QList<Object*> objs;
    for (auto i : items)
        objs << i->object();

    // group
    a = actions.addAction(tr("Group objects"), scene);
    a->setStatusTip(tr("Puts the selected objects into a group object"));
    connect(a, &QAction::triggered, [this, objs]()
    {
        editor->groupObjects(objs);
    });

}
*/

namespace {

    // for sorting the insert-object list
    bool sortObjectList_Priority(const Object * o1, const Object * o2)
    {
        return ObjectFactory::objectPriority(o1) > ObjectFactory::objectPriority(o2);
    }
}

QMenu * ObjectGraphScene::Private::createObjectsMenu(
        Object *parent, bool with_template, bool with_shortcuts, bool child_only, int groups)
{
    QList<const Object*> list;
    if (child_only)
        list = ObjectFactory::possibleChildObjects(parent);
    else
        list = ObjectFactory::objects(groups);

    if (list.empty() && !with_template)
        return 0;

    QMenu * menu = new QMenu();

    // from template
    if (with_template)
    {
        QAction * a = new QAction(tr("from template ..."), scene);
        a->setData("_template_");
        menu->addAction(a);
        menu->addSeparator();
    }

    if (!list.empty())
    {
        // sort by priority
        qStableSort(list.begin(), list.end(), sortObjectList_Priority);

        int curprio = ObjectFactory::objectPriority( list.front() );
        QMenu * sub = new QMenu(menu);
        sub->setTitle(ObjectFactory::objectPriorityName(curprio));
        menu->addMenu(sub);
        for (auto o : list)
        {
            if (curprio != ObjectFactory::objectPriority(o))
            {
                curprio = ObjectFactory::objectPriority(o);
                sub = new QMenu(menu);
                sub->setTitle(ObjectFactory::objectPriorityName(curprio));
                menu->addMenu(sub);
            }

            QAction * a = new QAction(AppIcons::iconForObject(o), o->name(), scene);
            a->setData(o->className());
            if (with_shortcuts)
            {
                if (o->className() == "AxisRotate")
                    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_R);
                if (o->className() == "Translate")
                    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_T);
                if (o->className() == "Scale")
                    a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_S);
            }
            sub->addAction(a);
        }
    }

    return menu;
}


// ------------------------------- editor signals ----------------------------------------

void ObjectGraphScene::onObjectAdded_(Object * o)
{
#ifdef QT_DEBUG
    //o->dumpAttachedData();
#endif
    if (!o->isVisible())
        return;

    const QPoint pos = o->hasAttachedData(Object::DT_GRAPH_POS)
                          ? o->getAttachedData(Object::DT_GRAPH_POS).toPoint()
                          : QPoint(1,1);

    p_->createObjectItem(o, pos);
    p_->recreateModulatorItems();
}

void ObjectGraphScene::onObjectsAdded_(const QList<Object*>& list)
{
    for (auto o : list)
    if (o->isVisible())
    {
        const QPoint pos = o->hasAttachedData(Object::DT_GRAPH_POS)
                          ? o->getAttachedData(Object::DT_GRAPH_POS).toPoint()
                          : QPoint(1,1);

        p_->createObjectItem(o, pos);
    }
    p_->recreateModulatorItems();
}

void ObjectGraphScene::onObjectDeleted_(const Object* o)
{
    Q_UNUSED(o);
    MO_DEBUG_GUI("ObjectGraphScene::onObjectDeleted(" << (void*)o << ")");

#if 1
    setRootObject(p_->root);
#else
    // remove items and references
    auto item = itemForObject(o);
    if (item)
    {
        removeItem(item);
        delete item;
    }
    p_->itemMap.erase(const_cast<Object*>(o));

    // recreate all modulation items
    p_->recreateModulatorItems();
#endif

    MO_DEBUG_GUI("ObjectGraphScene::onObjectDeleted(" << (void*)o << ") finished");
}

void ObjectGraphScene::onObjectsDeleted_(const QList<Object*>& )
{
    MO_DEBUG_GUI("ObjectGraphScene::onObjectsDeleted()");

    // again, don't bother to modify existing structure
    // just rebuild everything
    setRootObject(p_->root);
}


void ObjectGraphScene::onObjectMoved_(Object * , Object *)
{
    MO_DEBUG_GUI("ObjectGraphScene::onObjectMoved_()");

    // XXX Something's not right with below code
    // segfaults in AbstractObjectItem::mapToScene
    //  here's the shortcut
    setRootObject(p_->root);

/*  // remove items and references of previous item
    auto item = itemForObject(o);
    if (item)
    {
        removeItem(item);
        delete item;
    }
    p_->itemMap.erase(const_cast<Object*>(o));
    p_->modItemMap.erase(const_cast<Object*>(o));
    p_->conItemMap.erase(const_cast<Object*>(o));

    // create new item
    const QPoint pos = o->hasAttachedData(Object::DT_GRAPH_POS)
                          ? o->getAttachedData(Object::DT_GRAPH_POS).toPoint()
                          : QPoint(1,1);
    p_->createObjectItem(o, pos);

    p_->recreateModulatorItems();*/
}

void ObjectGraphScene::onObjectNameChanged_(Object * o)
{
    auto item = itemForObject(o);
    if (item)
        item->updateLabels();
}

void ObjectGraphScene::onObjectColorChanged_(Object * o)
{
    auto item = itemForObject(o);
    if (item)
        item->updateColors();
}

void ObjectGraphScene::onObjectChanged_(Object * o)
{
    auto item = itemForObject(o);
    if (item)
    {
        item->updateConnectors();
        item->updateLabels();
    }
}

void ObjectGraphScene::onModulatorAdded_(Modulator *)
{
    p_->recreateModulatorItems();
}

void ObjectGraphScene::onModulatorDeleted_()
{
    p_->recreateModulatorItems();
}

void ObjectGraphScene::onConnectionsChanged_()
{
    p_->recreateModulatorItems();
}

void ObjectGraphScene::onParameterVisibilityChanged_(Parameter * )
{
    p_->resolveLayout(true);
}

void ObjectGraphScene::onParameterChanged_(Parameter* p)
{
    auto obj = p->object();
    auto it = itemForObject(obj);
    if (it)
    {
        it->updateLabels();
        it->updateControlItems();
        // and repaint the whole, in case paramter was activity
        if (p->idName() == "_activescope")
            it->update();
    }
}

// ----------------------------------- editing -------------------------------------------

void ObjectGraphScene::addObject(
        Object *parent, Object *newObject, const QPoint& gridPos, int insert_index)
{
    MO_ASSERT(p_->root && p_->root->editor(), "Can't edit");

    newObject->setAttachedData(gridPos, Object::DT_GRAPH_POS);

    p_->nextSelectedObject = newObject;
    p_->root->editor()->addObject(parent, newObject, insert_index);
}

void ObjectGraphScene::addObjects(
        Object *parent, const QList<Object *> newObjects, const QPoint &gridPos, int insert_index)
{
    MO_ASSERT(p_->root && p_->root->editor(), "Can't edit");

    // find first place to insert
    QPoint po = gridPos;
    if (auto it = itemForObject(parent))
    {
        po = nextFreePosition(it, po);
    }
    // set pos for all items in advance
    for (auto o : newObjects)
    {
        o->setAttachedData(po, Object::DT_GRAPH_POS);
        ++po.ry();
    }

    p_->root->editor()->addObjects(parent, newObjects, insert_index);
}

void ObjectGraphScene::deleteObject(Object * o)
{
    MO_ASSERT(p_->root && p_->root->editor(), "Can't edit");

    MO_ASSERT(o != p_->root, "Can't delete root here");

    p_->root->editor()->deleteObject(o);
}

void ObjectGraphScene::deleteObjects(const QList<AbstractObjectItem *> items1)
{
    MO_ASSERT(p_->root && p_->root->editor(), "Can't edit");

    if (items1.isEmpty())
        return;

    auto items = items1;
    reduceToTopLevel(items);
/*
    qDebug() << "-------------\n"
             << items << "\n"
                << items1 << "\n"
                   << selectedItems() << "\n"
                      << selectedObjectItems() << "\n";
*/
    MO_ASSERT(items.first()->object() != p_->root, "Can't delete root here");

    QList<Object*> objs;
    for (auto i : items)
        objs << i->object();

    p_->root->editor()->deleteObjects(objs);
}

/*
void ObjectGraphScene::addModulator(Parameter * p, const QString &idName)
{
    if (!p_->root)
        return;

    p_->root->editor()->addModulator(p, idName);

    p_->recreateModulatorItems();
}
*/


// ------------------------------- clipboard ---------------------------------------------

QMimeData * ObjectGraphScene::mimeData(const QList<AbstractObjectItem *> & list) const
{
    // get objects
    QSet<Object*> objects;
    for (auto item : list)
        if (item->object())
            objects << item->object();

    // filter only top-level objects
    // we don't want to copy the children of a copied object
    QList<Object*> toplevel;
    for (Object * o : objects)
    {
        Object * tl = o;
        while ((o = o->parentObject()))
        {
            if (objects.contains(o))
            {
                tl = 0;
                break;
            }
        }

        if (tl)
            toplevel << tl;
    }

    auto data = new ObjectTreeMimeData();
    data->storeObjectTrees(toplevel);
    return data;
}

void ObjectGraphScene::dropMimeData(const QMimeData * data, const QPoint &gridPos)
{
    MO_DEBUG_GUI("ObjectGraphScene::dropMimeData()");

    if (!p_->root)
    {
        MO_WARNING("ObjectGraphScene::dropMimeData() without root object")
        return;
    }

    if (!data->formats().contains(ObjectTreeMimeData::objectMimeType))
    {
        MO_WARNING("ObjectGraphScene::dropMimeData() mimedata is invalid");
        return;
    }

    // NOTE: dynamic_cast or dynamic_cast won't work between
    // application boundaries, e.g. after quit or pasting into
    // a different instance. But static_cast works alright after we checked the MimeType.
    // It's important to not rely on class members of ObjectTreeMimeData
    // but to manage everything per QMimeData::data()
    auto objdata = static_cast<const ObjectTreeMimeData*>(data);

    // deserialize object
    QList<Object*> copies = objdata->getObjectTrees();

    // XXX how to copy/paste gui settings???
    // UPDATE: Almost all settings are now saved in Object::attachedData()

    // find object to paste in
    Object * root = p_->root;
    if (auto pitem = objectItemAt(gridPos))
        if (pitem->object())
            root = pitem->object();

    QPoint objPos = QPoint(0,0);
    if (root != p_->root)
        if (auto item = itemForObject(root))
            objPos = item->globalGridPos();

    addObjects(root, copies, gridPos - objPos);

}








} // namespace GUI
} // namespace MO
