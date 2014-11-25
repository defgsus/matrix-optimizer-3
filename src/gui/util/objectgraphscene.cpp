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
#include "gui/util/scenesettings.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/param/modulator.h"
#include "object/util/objectmodulatorgraph.h"
#include "object/objectfactory.h"
#include "object/scenelock_p.h"
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
        :   gui     (0),
            scene   (scene),
            zStack  (0),
            action  (A_NONE)
    { }

    /// Creates the item for o and all of its children
    void createObjectItem(Object * o, const QPoint &grid_pos);
    /// Creates the items for all children of o and recusively
    void createObjectChildItems(Object * o, AbstractObjectItem * item);
    void createModulatorItems(Object * root);
    void addModItem(Modulator *);
    void addModItemMap(Object *, ModulatorItem *);
    /// Recursively get the item below @p localGridPos
    AbstractObjectItem * childItemAt(AbstractObjectItem * parent, const QPoint& localGridPos);
    void raiseModItems(AbstractObjectItem*); ///< raise all ModulatorItems of the item and it's childs
    void resolveLayout();

    void clearActions();
    void showPopup(); ///< Runs popup after actions have been created
    void createNewObjectMenu(Object * o);
    void createClipboardMenu(Object * parent, const QList<AbstractObjectItem*>& items);
    QMenu * createObjectsMenu(Object *parent, bool with_template, bool with_shortcuts);

    SceneSettings * gui;
    ObjectGraphScene * scene;
    Scene * root;
    std::map<Object*, AbstractObjectItem*> itemMap;
    std::multimap<Object*, QList<ModulatorItem*>> modItemMap;
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
    connect(this, SIGNAL(changed(QList<QRectF>)), this, SLOT(onChanged_()));
}

ObjectGraphScene::~ObjectGraphScene()
{
    delete p_;
}

AbstractObjectItem * ObjectGraphScene::itemForObject(Object *o) const
{
    auto i = p_->itemMap.find(o);
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

void ObjectGraphScene::setGuiSettings(SceneSettings *set)
{
    p_->gui = set;
}

SceneSettings * ObjectGraphScene::guiSettings() const
{
    return p_->gui;
}

void ObjectGraphScene::setRootObject(Object *root)
{
    clear();

    p_->itemMap.clear();
    p_->modItemMap.clear();
    p_->modItems.clear();
    p_->zStack = 0;
    p_->root = qobject_cast<Scene*>(root);

    p_->createObjectChildItems(root, 0);
    p_->resolveLayout();
    p_->createModulatorItems(root);

    //for (auto & p : p_->itemMap)
    //    MO_DEBUG(p.first->name() << " :: " << p.second);
}



void ObjectGraphScene::setGridPos(AbstractObjectItem * item, const QPoint &gridPos)
{
    item->setGridPos(gridPos);
    // save in gui
    if (item->object() && guiSettings())
        guiSettings()->setLocalGridPos(item->object(), "0", gridPos);
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

        // set expanded according to gui settings
        if (gui && gui->getExpanded(c, "0"))
            item->setExpanded(true);

        // set initial position (from gui settings or vertically)
        if (gui && gui->hasLocalGridPos(c, "0"))
        {
            item->setGridPos(gui->getLocalGridPos(c, "0"));
            y = std::max(y, item->gridPos().y());
        }
        else
            item->setGridPos(QPoint(1, y));

        // install in item tree
        if (!pitem)
            scene->addItem(item);
        else
            item->setParentItem(pitem);
        item->setZValue(++zStack);

        // add childs
        createObjectChildItems(c, item);

        y ++;
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

void ObjectGraphScene::Private::createObjectItem(Object *o, const QPoint& pos1)
{
    // create item
    auto item = new AbstractObjectItem(o);
    // save in map
    itemMap.insert(std::make_pair(o, item));

    // item for parent
    auto pitem = scene->itemForObject(o->parentObject());

    // make pos1 local to parent item
    QPoint pos(pos1);
    if (pitem)
    {
        pos = scene->mapToGrid(
                    pitem->mapFromScene(
                        scene->mapFromGrid(pos)));
        pos.rx() = std::max(1, pos.x());
        pos.ry() = std::max(1, pos.y());
    }

    // set initial position
    item->setGridPos(pos);
    item->setZValue(++zStack);

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

QList<AbstractObjectItem*> ObjectGraphScene::topLevelObjectItems() const
{
    QList<AbstractObjectItem*> ret;

    for (auto & i : p_->itemMap)
        if (!i.second->parentItem())
            ret << i.second;

    return ret;
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
        // popup for empty space
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
    QString title(obj->name());
    p_->actions.addTitle(title, this);

    p_->actions.addSeparator(this);

        // new object
        p_->createNewObjectMenu(obj);

    p_->actions.addSeparator(this);

        // clipboard
        p_->createClipboardMenu(obj, items);

    p_->showPopup();
}

void ObjectGraphScene::Private::createNewObjectMenu(Object * obj)
{
    if (!root)
    {
        MO_WARNING("Can't edit");
        return;
    }

    if (!obj)
        obj = root;

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
            scene->addObject(obj, onew, popupGridPos);
    });

}

void ObjectGraphScene::Private::createClipboardMenu(Object * /*parent*/, const QList<AbstractObjectItem*>& items)
{
    const bool plural = items.size() > 1;
    Object * obj = items.isEmpty() ? 0 : items.first()->object();

    QAction * a;

    // copy
    if (obj && obj != root)
    {
        a = actions.addAction(plural ? tr("Copy objects") : tr("Copy"), scene);
        a->setStatusTip(tr("Copies the selected object(s) and all it's children to the clipboard"));
        a->setShortcut(Qt::CTRL + Qt::Key_C);
        connect(a, &QAction::triggered, [=]()
        {
            application->clipboard()->setMimeData(scene->mimeData(items));
        });
    }

    if (!plural)
    {
        const auto data = application->clipboard()->mimeData();
        const bool isClip = data->formats().contains(ObjectTreeMimeData::objectMimeType);

        if (isClip)
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


// ----------------------------------- editing -------------------------------------------

void ObjectGraphScene::addObject(Object *parent, Object *newObject, const QPoint& gridPos, int insert_index)
{
    MO_ASSERT(p_->root, "Can't edit");

    QString error;
    if (!parent->isSaveToAdd(newObject, error))
    {
        delete newObject;
        QMessageBox::critical(0, tr("can't add object"),
                              tr("The object %1 could not be added to %2.\n%3")
                              .arg(newObject->name())
                              .arg(parent->name())
                              .arg(error));
        return;
    }

    // add to object tree
    p_->root->addObject(parent, newObject, insert_index);
    // add to graphics scene
    p_->createObjectItem(newObject, gridPos);
}

void ObjectGraphScene::addObjects(Object *parent, const QList<Object *> newObjects, const QPoint &gridPos, int insert_index)
{
    MO_ASSERT(p_->root, "Can't edit");

    QString error;

    int k = 0;
    for (auto o : newObjects)
    {
        QString err;
        if (!parent->isSaveToAdd(o, err))
        {
            delete o;
            error += "\n" + err;
        }
        else
            addObject(parent, o, gridPos + QPoint(0, k++), insert_index);
    }

    if (!error.isEmpty())
    {
        QMessageBox::critical(0, tr("can't add object"),
                              tr("Some objects could not be added to %1.%2")
                              .arg(parent->name())
                              .arg(error));
    }
}

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

    addObjects(root, copies, gridPos);

}


} // namespace GUI
} // namespace MO
