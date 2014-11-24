/** @file objectgraphscene.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#include <map>

#include "objectgraphscene.h"
#include "gui/item/abstractobjectitem.h"
#include "gui/item/modulatoritem.h"
#include "object/object.h"
#include "object/param/modulator.h"
#include "object/util/objectmodulatorgraph.h"
#include "io/log.h"

namespace MO {
namespace GUI {

class ObjectGraphScene::Private
{
public:
    Private(ObjectGraphScene * scene)
        :   scene   (scene)
    { }

    void createObjectChildItems(Object * o, AbstractObjectItem * item);
    void createModulatorItems(Object * root);
    void addModItem(Modulator *);

    void resolveLayout();

    ObjectGraphScene * scene;
    std::map<Object*, AbstractObjectItem*> itemMap;
    std::multimap<Object*, ModulatorItem*> modItemMap;
    QList<ModulatorItem*> modItems;
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

void ObjectGraphScene::setRootObject(Object *root)
{
    clear();

    p_->itemMap.clear();
    p_->modItemMap.clear();
    p_->modItems.clear();

    p_->createObjectChildItems(root, 0);
    p_->resolveLayout();
    p_->createModulatorItems(root);

    //for (auto & p : p_->itemMap)
    //    MO_DEBUG(p.first->name() << " :: " << p.second);
}

void ObjectGraphScene::setGridPos(AbstractObjectItem * item, const QPoint &gridPos)
{
    item->setGridPos(gridPos);
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

        // set initial position
        item->setGridPos(QPoint(1, y));

        // install in item tree
        if (!pitem)
            scene->addItem(item);
        else
            item->setParentItem(pitem);

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
    item->updateShape(); // calc arrow shape

    // store in map
    modItems.append( item );
    //modItemMap.insert(std::make_pair(m->parent(), item));
    //modItemMap.insert(std::make_pair(m->modulator(), item));
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

void ObjectGraphScene::repaintModulators(AbstractObjectItem * item)
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

} // namespace GUI
} // namespace MO
