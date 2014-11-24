/** @file objectgraphscene.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#include "objectgraphscene.h"
#include "gui/item/abstractobjectitem.h"
#include "object/object.h"
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

    void resolveLayout();

    ObjectGraphScene * scene;
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

void ObjectGraphScene::setRootObject(Object *root)
{
    clear();

    p_->createObjectChildItems(root, 0);

    p_->resolveLayout();
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
        auto item = new AbstractObjectItem(c);
        item->setGridPos(QPoint(1, y));

        if (!pitem)
            scene->addItem(item);
        else
            item->setParentItem(pitem);

        createObjectChildItems(c, item);

        y ++;
    }
}

void ObjectGraphScene::Private::resolveLayout()
{
    const auto list = scene->items();

    // set all item grid sizes according to their childs
    for (QGraphicsItem * item : list)
    if (item->type() >= AbstractObjectItem::T_BASE)
    {
        auto o = static_cast<AbstractObjectItem *>(item);

        // get dirty root items
        if (!o->isLayouted() && !o->parentItem())
            o->adjustSizeToChildren();
    }
}



} // namespace GUI
} // namespace MO
