/** @file frontscene.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QAction>
#include <QCursor>
#include <QGraphicsView>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include "frontscene.h"
#include "gui/item/frontgroupitem.h"
#include "gui/item/frontfloatitem.h"
#include "object/object.h"
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parameterfilename.h"
#include "object/param/parametertimeline1d.h"
#include "io/xmlstream.h"
#include "io/log.h"

namespace MO {
namespace GUI {


struct FrontScene::Private
{
    Private(FrontScene * s)
        : gscene    (s)
        , editMode  (false)
    { }

    void createDefaultActions();

    /** Adds the item to the graphicsscene if necessary
        and most importantly initializes the item to the current settings.
        Also recurses through children.
        @note Always use this function to add items! */
    void addItem(AbstractFrontItem * item);

    /** Recusively creates all items for the object(s) */
    void createItems(Object * root, const QPointF& pos = QPointF(0,0),
                                    AbstractFrontItem * parent = 0);

    FrontScene * gscene;

    bool editMode;

};



FrontScene::FrontScene(QObject *parent)
    : QGraphicsScene    (parent)
    , p_                (new Private(this))
{
    connect(this, SIGNAL(selectionChanged()),
            this, SLOT(onSelectionChanged_()));
}

FrontScene::~FrontScene()
{
    delete p_;
}

void FrontScene::serialize(IO::XmlStream & io) const
{
    io.newSection("user-interface");

        io.write("version", 1);

        auto list = topLevelFrontItems();
        for (AbstractFrontItem * i : list)
        {
            i->serialize(io);
        }

    io.endSection();
}

void FrontScene::deserialize(IO::XmlStream & io)
{
    io.verifySection("user-interface");

        clear();

        const int ver = io.expectInt("version");
        Q_UNUSED(ver);

        while (io.nextSubSection())
        {
            if (io.section() == "interface-item")
            {
                auto item = AbstractFrontItem::deserialize(io);
                p_->addItem(item);
            }

            io.leaveSection();
        }
}

void FrontScene::saveXml(const QString &filename) const
{
    IO::XmlStream xml;
    xml.startWriting();
    serialize(xml);
    xml.stopWriting();
    xml.save(filename);
}

void FrontScene::loadXml(const QString &filename)
{
    IO::XmlStream xml;
    xml.load(filename);
    xml.startReading();
    while (xml.nextSubSection())
    {
        if (xml.section() == "user-interface")
            deserialize(xml);
        xml.leaveSection();
    }
    xml.stopReading();
}


bool FrontScene::editMode() const
{
    return p_->editMode;
}

void FrontScene::setEditMode(bool e)
{
    if (p_->editMode == e)
        return;
    p_->editMode = e;
    auto list = frontItems();
    for (auto i : list)
        i->setEditMode(e);
    update();
}

void FrontScene::setRootObject(Object *root)
{
    //clear();

    //p_->createItems(root);
}

void FrontScene::Private::addItem(AbstractFrontItem *item)
{
    if (!item->parentItem() || item->parentItem()->scene() != gscene)
        gscene->addItem(item);

    // init
    item->setEditMode(editMode);

    // children
    auto childs = item->childFrontItems();
    for (auto c : childs)
        addItem(c);
}

void FrontScene::Private::createItems(Object *root, const QPointF& pos, AbstractFrontItem *parent)
{
    qreal maxh = pos.y();
    auto localPos = pos;
    for (Parameter * p : root->params()->parameters())
    {
        if (!p->isVisibleInterface())
            continue;
/*
        // create/add item
        auto item = new AbstractFrontItem(p, parent);
        item->setPos(localPos);
        addItem(item);

        // keep track of insert pos
        // XXX bullshit))
        localPos.rx() += item->rect().width() + 4.;
        maxh = std::max(maxh, item->rect().bottom() + 4.);
        */
    }

    localPos.ry() = maxh;

    // recurse
    for (Object * c : root->childObjects())
        createItems(c, pos, 0);
}

AbstractFrontItem * FrontScene::createNew(FrontItemType type,
                                          QGraphicsItem * parent, const QPointF& pos)
{
    AbstractFrontItem * item;

    switch (type)
    {
        default:
        case FIT_GROUP: item = new FrontGroupItem(parent); break;
        case FIT_FLOAT: item = new FrontFloatItem(parent); break;
    }

    item->setPos(pos);
    p_->addItem(item);
    return item;
}

void FrontScene::groupItems(const QList<AbstractFrontItem *> &items)
{
    if (items.isEmpty())
        return;

    /** @todo find common parent */
    auto group = createNew(FIT_GROUP, 0);

    QRectF r;
    for (AbstractFrontItem * i : items)
    {
        QRectF b = i->mapToScene( i->boundingRect() ).boundingRect();
        r |= b;
        i->setPos(b.topLeft());
    }

    group->setInsidePos(r.topLeft());
    group->setSize(r.size());

    for (AbstractFrontItem * i : items)
    {
        i->setParentItem(group);
        i->setPos(group->mapFromScene(i->pos()));
    }
}

QPointF FrontScene::cursorPos() const
{
    QPoint p = QCursor::pos();
    auto list = views();
    for (QGraphicsView * v : list)
    {
        QPoint lp = v->mapFromGlobal(p);
        if (v->rect().contains(lp))
            return v->mapToScene(lp);
    }
    return QPointF(0,0);
}

QRectF FrontScene::viewsRect() const
{
    QRectF r;
    auto list = views();
    for (QGraphicsView * v : list)
        r |= v->sceneRect();
    return r;
}

QList<AbstractFrontItem*> FrontScene::selectedFrontItems() const
{
    auto list = selectedItems();
    if (list.isEmpty())
        return QList<AbstractFrontItem*>();

    QList<AbstractFrontItem*> fitems;
    for (QGraphicsItem * item : list)
        if (auto a = dynamic_cast<AbstractFrontItem*>(item))
            fitems << a;
    return fitems;
}

QList<AbstractFrontItem*> FrontScene::frontItems() const
{
    auto list = items();
    if (list.isEmpty())
        return QList<AbstractFrontItem*>();

    QList<AbstractFrontItem*> fitems;
    for (QGraphicsItem * item : list)
        if (auto a = dynamic_cast<AbstractFrontItem*>(item))
            fitems << a;
    return fitems;
}

QList<AbstractFrontItem*> FrontScene::topLevelFrontItems() const
{
    auto list = items();
    if (list.isEmpty())
        return QList<AbstractFrontItem*>();

    QList<AbstractFrontItem*> fitems;
    for (QGraphicsItem * item : list)
        if (item->parentItem() == 0)
            if (auto a = dynamic_cast<AbstractFrontItem*>(item))
                fitems << a;
    return fitems;
}



void FrontScene::onSelectionChanged_()
{
    auto fitems = selectedFrontItems();
    if (fitems.isEmpty())
        emit itemUnselected();
    else if (fitems.size() == 1)
        emit itemSelected(fitems[0]);
    else
        emit itemsSelected(fitems);
}



QList<QAction*> FrontScene::createDefaultActions()
{
    QList<QAction*> list;
    QAction * a;
    list.push_back( a = new QAction(tr("Edit"), this) );
    a->setCheckable(true);
    a->setChecked(editMode());
    a->setShortcut(Qt::ALT + Qt::Key_E);
    connect(a, &QAction::triggered, [=]() { setEditMode(!editMode()); });

    list.push_back( a = new QAction(tr("New group"), this) );
    a->setShortcut(Qt::CTRL + Qt::Key_1);
    connect(a, &QAction::triggered, [=]() { if (editMode()) createNew(FIT_GROUP, 0, cursorPos()); });

    list.push_back( a = new QAction(tr("New float control"), this) );
    a->setShortcut(Qt::CTRL + Qt::Key_2);
    connect(a, &QAction::triggered, [=]() { if (editMode()) createNew(FIT_FLOAT, 0, cursorPos()); });

    list.push_back( a = new QAction(tr("Group selected items"), this) );
    a->setShortcut(Qt::ALT + Qt::Key_G);
    connect(a, &QAction::triggered, [=]() { if (editMode()) groupItems(selectedFrontItems()); });

    return list;
}

namespace { const qreal FAR = 10000000.; }

void FrontScene::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    if (editMode())
    {
        //update(viewsRect());
        update();
    }

    QGraphicsScene::mouseMoveEvent(e);
}

void FrontScene::drawForeground(QPainter *p, const QRectF &)
{
    if (editMode())
    {
        auto cpos = cursorPos();

        p->setPen(QPen(QColor(255,255,255,100)));
        p->drawLine(-FAR, cpos.y(), FAR, cpos.y());
        p->drawLine(cpos.x(), -FAR, cpos.x(), FAR);
    }
}

} // namespace GUI
} // namespace MO
