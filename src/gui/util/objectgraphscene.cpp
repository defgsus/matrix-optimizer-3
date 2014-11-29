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

#include "objectgraphscene.h"
#include "gui/item/abstractobjectitem.h"
#include "gui/item/modulatoritem.h"
#include "gui/util/objectgraphsettings.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/param/modulator.h"
#include "object/util/objectmodulatorgraph.h"
#include "object/objectfactory.h"
#include "object/util/objecteditor.h"
#include "model/objecttreemimedata.h"
#include "tool/actionlist.h"
#include "io/application.h"
#include "io/log.h"

namespace MO {
namespace GUI {

class ObjectGraphScene::Private
{
public:
    enum Action
    {
        A_NONE,
        A_DRAG_SPACE,
        A_RECT_SELECT
    };

    Private(ObjectGraphScene * scene)
        :   scene   (scene),
            editor  (0),
            zStack  (0),
            action  (A_NONE)
    { }

    /// Creates the items for o and all of its children
    void createObjectItem(Object * o, const QPoint &local_pos);
    /// Creates the items for all children of o and recursively
    void createObjectChildItems(Object * o, AbstractObjectItem * item);
    void createModulatorItems(Object * root);
    /// Completely recreates all cables (to nasty to do it selectively)
    void recreateModulatorItems();
    void addModItem(Modulator *);
    void addModItemMap(Object *, ModulatorItem *);
    /// Removes all modulator items of object and it's children
    //void removeModItemsFor(Object *);
    /// Returns all modulator items of object and it's children
    QList<ModulatorItem*> getAllModItemsFor(Object *);
    void getAllModItemsFor(Object * o, QSet<ModulatorItem*>& set);
    /// Recursively get the item below @p localGridPos
    AbstractObjectItem * childItemAt(AbstractObjectItem * parent, const QPoint& localGridPos);
    void raiseModItems(AbstractObjectItem*); ///< raise all ModulatorItems of the item and it's childs
    /// Resizes the items to fit their children and updates cable positions
    void resolveLayout();

    void clearActions();
    void showPopup(); ///< Runs popup after actions have been created
    void createNewObjectMenu(Object * o);
    void createClipboardMenu(Object * parent, const QList<AbstractObjectItem*>& items);
    QMenu * createObjectsMenu(Object *parent, bool with_template, bool with_shortcuts);

    ObjectGraphScene * scene;
    Scene * root;
    ObjectEditor * editor;
    std::map<Object*, AbstractObjectItem*> itemMap;
    std::map<Object*, QList<ModulatorItem*>> modItemMap;
    QList<ModulatorItem*> modItems;
    int zStack; ///< highest current stack value
    Action action;
    QPointF lastMousePos;
    QRectF selUpdateRect;

    ActionList actions;
    QPoint popupGridPos;
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
    p_->modItems.clear();
    p_->zStack = 0;
    p_->root = qobject_cast<Scene*>(root);

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
            connect(p_->editor, SIGNAL(objectDeleted(const MO::Object*)),
                    this, SLOT(onObjectDeleted_(const MO::Object*)));
            connect(p_->editor, SIGNAL(modulatorAdded(MO::Modulator*)),
                    this, SLOT(onModulatorAdded_(MO::Modulator*)));
            connect(p_->editor, SIGNAL(modulatorDeleted(const MO::Modulator*)),
                    this, SLOT(onModulatorDeleted_(const MO::Modulator*)));
        }
    }

    p_->createObjectChildItems(root, 0);
    p_->resolveLayout();
    p_->createModulatorItems(root);


    //for (auto & p : p_->itemMap)
    //    MO_DEBUG(p.first->name() << " :: " << p.second);
}



void ObjectGraphScene::setGridPos(AbstractObjectItem * item, const QPoint &gridPos)
{
    item->setGridPos(gridPos);
    /*// save in object
    if (item->object())
        item->object()->setAttachedData(gridPos, Object::DT_GRAPH_POS);
        */
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


void ObjectGraphScene::onChanged_()
{
    // XXX this comes quite often, even for button-less mouse-moves!!
    //MO_DEBUG("changed");
    p_->resolveLayout();
}

void ObjectGraphScene::Private::createObjectChildItems(Object *o, AbstractObjectItem * pitem)
{
    int y = 1;
    for (Object * c : o->childObjects())
    {
        // create item
        auto item = new AbstractObjectItem(c);
        // save in map
        itemMap.insert(std::make_pair(c, item));

        //o->dumpAttachedData();

        // set expanded according to gui settings
        if (c->hasAttachedData(Object::DT_GRAPH_EXPANDED))
            item->setExpanded(c->getAttachedData(Object::DT_GRAPH_EXPANDED).toBool());

        // set initial position (from gui settings or top-to-bottom)
        if (c->hasAttachedData(Object::DT_GRAPH_POS))
            item->setGridPos(c->getAttachedData(Object::DT_GRAPH_POS).toPoint());
        else
            item->setGridPos(QPoint(1, y));

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
        addModItem(m);
    }

    // process childs
    for (auto c : root->childObjects())
        createModulatorItems(c);
}

void ObjectGraphScene::Private::addModItem(Modulator * m)
{
    auto item = new ModulatorItem(m);
    scene->addItem( item );
    item->setZValue(++zStack);
    item->updateShape(); // calc arrow shape

    // store in map
    modItems.append( item );
    // ModulatorItems per object
    addModItemMap(m->parent(), item);
    addModItemMap(m->modulator(), item);
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

void ObjectGraphScene::Private::createObjectItem(Object *o, const QPoint& local_pos)
{
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

    item->setGridPos(pos);
    item->setZValue(++zStack);

    const bool isExpand = o->hasAttachedData(Object::DT_GRAPH_EXPANDED)
                        && o->getAttachedData(Object::DT_GRAPH_EXPANDED).toBool();
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
}

void ObjectGraphScene::Private::resolveLayout()
{
    const auto list = scene->items();

    bool change = false;

    // set all item grid sizes according to their childs
    for (QGraphicsItem * item : list)
    if (item->type() >= AbstractObjectItem::T_BASE)
    {
        auto o = static_cast<AbstractObjectItem *>(item);

        // get dirty root items
        if (!o->isLayouted() && !o->parentItem())
        {
            o->adjustSizeToChildren();
            o->setLayoutDirty(false);
            change = true;
        }
    }

    // update modulator items
    if (change)
    for (QGraphicsItem * item : list)
    if (item->type() == ModulatorItem::Type)
    {
        auto m = static_cast<ModulatorItem*>(item);
        m->updateShape();
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
    auto i = modItemMap.find(item->object());
    if (i != modItemMap.end())
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

QList<ModulatorItem*> ObjectGraphScene::Private::getAllModItemsFor(Object * o)
{
    QSet<ModulatorItem*> set;
    getAllModItemsFor(o, set);
    return set.toList();
}

void ObjectGraphScene::Private::getAllModItemsFor(Object * o, QSet<ModulatorItem*>& set)
{
    auto i = modItemMap.find(o);
    if (i != modItemMap.end())
    {
        for (auto mi : i->second)
            set.insert(mi);
    }

    for (auto c : o->childObjects())
        getAllModItemsFor(c, set);
}
/*
void ObjectGraphScene::Private::removeModItemsFor(Object * o)
{
    auto moditems = getAllModItemsFor(o);

    // remove entries in map
    QList<Object*> allsubs = o->findChildObjects(Object::TG_ALL, true);
    allsubs << o;
    for (auto o : allsubs)
        modItemMap.erase(o);

    // delete items
    for (auto mi : moditems)
    {
        modItems.removeAll(mi);
        scene->removeItem(mi);
        delete mi;
    }
}
*/

void ObjectGraphScene::Private::recreateModulatorItems()
{
    for (auto m : modItems)
    {
        scene->removeItem(m);
        delete m;
    }
    modItems.clear();
    modItemMap.clear();

    resolveLayout();

    if (root)
        createModulatorItems(root);
}

QList<AbstractObjectItem*> ObjectGraphScene::topLevelObjectItems() const
{
    QList<AbstractObjectItem*> ret;

    for (auto & i : p_->itemMap)
        if (!i.second->parentItem())
            ret << i.second;

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

QPoint ObjectGraphScene::nextFreePosition(Object *parent, const QPoint &pos1) const
{
    auto item = itemForObject(parent);
    if (!item)
        return pos1;

    QPoint pos(pos1);
    while (p_->childItemAt(item, pos))
        ++pos.ry();

    return pos;
}

AbstractObjectItem * ObjectGraphScene::Private::childItemAt(AbstractObjectItem * parent, const QPoint& pos)
{
    // pos is local in parent

    if (pos.x() < 0 || pos.y() < 0
        || pos.x() >= parent->gridSize().width()
        || pos.y() >= parent->gridSize().height())
        return 0;

    for (auto i : parent->childItems())
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

// ------------------------------ mouse events ----------------------------------

void ObjectGraphScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
/*    auto it = objectItemAt(mapToGrid(event->scenePos()));
    if (it)
        MO_DEBUG(it->object()->name());
*/

    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted())
        return;

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
}

void ObjectGraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
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
        p_->selUpdateRect.setLeft(  std::min(p_->lastMousePos.x(), event->scenePos().x()));
        p_->selUpdateRect.setRight( std::max(p_->lastMousePos.x(), event->scenePos().x()));
        p_->selUpdateRect.setTop(   std::min(p_->lastMousePos.y(), event->scenePos().y()));
        p_->selUpdateRect.setBottom(std::max(p_->lastMousePos.y(), event->scenePos().y()));
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
        return;

    if (p_->action == Private::A_RECT_SELECT)
    {
        update(p_->selUpdateRect);
        //setSelectionArea(QPainterPath());
    }

    p_->action = Private::A_NONE;
}





// ------------------------------------- drawing -------------------------------

void ObjectGraphScene::drawForeground(QPainter *p, const QRectF &)
{
    if (p_->action == Private::A_RECT_SELECT)
    {
        p->setBrush(Qt::NoBrush);
        p->setPen(ObjectGraphSettings::penSelectionFrame());

        p->drawPath(selectionArea());
    }
}


// --------------------------------- menu --------------------------------------

void ObjectGraphScene::Private::showPopup()
{
    auto popup = new QMenu(application->mainWindow());
    popup->setAttribute(Qt::WA_DeleteOnClose, true);

    popup->addActions(actions);

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

    // new object
    if (items.size() < 2)
        p_->createNewObjectMenu(obj);

    // clipboard
    p_->createClipboardMenu(obj, items);

    p_->showPopup();
}

void ObjectGraphScene::Private::createNewObjectMenu(Object * obj)
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

}

void ObjectGraphScene::Private::createClipboardMenu(Object * /*parent*/, const QList<AbstractObjectItem*>& items)
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
            application->clipboard()->setMimeData(scene->mimeData(items));
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
        const auto data = application->clipboard()->mimeData();
        const bool isClipboard = data->formats().contains(ObjectTreeMimeData::objectMimeType);

        if (isClipboard)
        {
            const auto pitem = scene->objectItemAt(popupGridPos);
            const QString pname = pitem && pitem->object()
                    ? pitem->object()->name() : tr("Scene");

            // paste
            a = actions.addAction(tr("Paste into %1").arg(pname), scene);
            connect(a, &QAction::triggered, [=]()
            {
                scene->dropMimeData(application->clipboard()->mimeData(), popupGridPos);
            });
        }
    }
}

namespace {

    // for sorting the insert-object list
    bool sortObjectList_Priority(const Object * o1, const Object * o2)
    {
        return Object::objectPriority(o1) > Object::objectPriority(o2);
    }
}

QMenu * ObjectGraphScene::Private::createObjectsMenu(Object *parent, bool with_template, bool with_shortcuts)
{
    QList<const Object*> list(ObjectFactory::possibleChildObjects(parent));
    if (list.empty() && !with_template)
        return 0;

    // sort by priority
    qStableSort(list.begin(), list.end(), sortObjectList_Priority);

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
        int curprio = Object::objectPriority( list.front() );
        for (auto o : list)
        {
            if (curprio != Object::objectPriority(o))
            {
                menu->addSeparator();
                curprio = Object::objectPriority(o);
            }

            QAction * a = new QAction(ObjectFactory::iconForObject(o), o->name(), scene);
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
            menu->addAction(a);
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

    const QPoint pos = o->hasAttachedData(Object::DT_GRAPH_POS)
                          ? o->getAttachedData(Object::DT_GRAPH_POS).toPoint()
                          : QPoint(1,1);

    p_->createObjectItem(o, pos);
    p_->recreateModulatorItems();
}

void ObjectGraphScene::onObjectDeleted_(const Object *o)
{
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
}

void ObjectGraphScene::onModulatorAdded_(Modulator *)
{
    p_->recreateModulatorItems();
}

void ObjectGraphScene::onModulatorDeleted_(const Modulator *)
{
    p_->recreateModulatorItems();
}

// ----------------------------------- editing -------------------------------------------

void ObjectGraphScene::addObject(Object *parent, Object *newObject, const QPoint& gridPos, int insert_index)
{
    MO_ASSERT(p_->root && p_->root->editor(), "Can't edit");

    newObject->setAttachedData(gridPos, Object::DT_GRAPH_POS);

    p_->root->editor()->addObject(parent, newObject, insert_index);
}

void ObjectGraphScene::addObjects(Object *parent, const QList<Object *> newObjects, const QPoint &gridPos, int insert_index)
{
    MO_ASSERT(p_->root && p_->root->editor(), "Can't edit");

    // set pos for all items
    int k=0;
    for (auto o : newObjects)
        o->setAttachedData(gridPos + QPoint(0, k++), Object::DT_GRAPH_POS);

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

    auto items = items1;
    reduceToTopLevel(items);

    for (auto item : items)
    {
        Object * o = item->object();
        MO_ASSERT(o != p_->root, "Can't delete root here");

        p_->root->editor()->deleteObject(o);
    }
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

    // NOTE: dynamic_cast or qobject_cast won't work between
    // application boundaries, e.g. after quit or pasting into
    // a different instance. But static_cast works alright after we checked the MimiType.
    // It's important to not rely on class members of ObjectTreeMimeData
    // but to manage everything per QMimeData::data()
    auto objdata = static_cast<const ObjectTreeMimeData*>(data);

    // deserialize object
    QList<Object*> copies = objdata->getObjectTrees();

    // XXX how to copy/paste gui settings???

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
