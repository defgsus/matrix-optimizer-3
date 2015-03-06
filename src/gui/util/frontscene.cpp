/** @file frontscene.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QAction>
#include <QMenu>
#include <QCursor>
#include <QGraphicsView>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMimeData>
#include <QClipboard>
#include <QMessageBox>

#include "frontscene.h"
#include "frontpreset.h"
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
#include "object/util/objecteditor.h"
#include "math/functions.h"
#include "tool/actionlist.h"
#include "io/application.h"
#include "io/currenttime.h"
#include "io/xmlstream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {


struct FrontScene::Private
{
    Private(FrontScene * s)
        : gscene    (s)
        , editor    (0)
        , editMode  (false)
    {
    }

    //void createDefaultActions();
    QMenu * createNewItemMenu(const QPointF& insertPos);

    /** Adds the item to the graphicsscene if necessary
        and most importantly initializes the item to the current scene and settings.
        Also recurses through children.
        @note Always use this function to add items!
        It's save to set the item's parent beforehand. */
    void addItem(AbstractFrontItem * item);

    /** Puts the @p item and all of it's children and sub-children into @p set */
    static void getAllItems(AbstractFrontItem * item, QSet<AbstractFrontItem*>& set);

    /* Recusively creates all items for the object(s) */
    //void createItems(Object * root, const QPointF& pos = QPointF(0,0),
    //                                AbstractFrontItem * parent = 0);

    /** (Re-)Creates all context menu edit actions for the current selection */
    void createEditActions();

    FrontScene * gscene;

    ObjectEditor * editor;
    bool editMode;
    ActionList editActions;

    QSet<QString> usedIds;
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
    auto list = topLevelFrontItems();
    serialize(io, list);
}

void FrontScene::serialize(IO::XmlStream & io, const QList<AbstractFrontItem*>& list) const
{
    MO_ASSERT(list.size() == reduceToCommonParent(list).size(),
              "Only top-level items allowed in FrontScene::serialize");

    io.newSection("user-interface");

        io.write("version", 1);

        for (AbstractFrontItem * i : list)
        {
            i->serialize(io);
        }

    io.endSection();
}

void FrontScene::deserialize(IO::XmlStream & io)
{
    QList<AbstractFrontItem*> list;

    deserialize(io, list);

    clearInterface();

    // add the items to scene
    for (auto i : list)
        p_->addItem(i);
}

void FrontScene::deserialize(IO::XmlStream& io, QList<AbstractFrontItem*>& list) const
{
    io.verifySection("user-interface");

    const int ver = io.expectInt("version");
    Q_UNUSED(ver);

    while (io.nextSubSection())
    {
        if (io.section() == "interface-item")
        {
            auto item = AbstractFrontItem::deserialize(io);
            list << item;
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

QString FrontScene::toXml() const
{
    IO::XmlStream xml;
    xml.startWriting();
    serialize(xml);
    xml.stopWriting();
    return xml.data();
}

void FrontScene::setXml(const QString & xmls)
{
    IO::XmlStream xml;
    xml.setData(xmls);
    xml.startReading();
    while (xml.nextSubSection())
    {
        if (xml.section() == "user-interface")
            deserialize(xml);
        xml.leaveSection();
    }
    xml.stopReading();
}

bool FrontScene::isEditMode() const
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

void FrontScene::setObjectEditor(ObjectEditor *e)
{
    p_->editor = e;
}

ObjectEditor * FrontScene::objectEditor() const
{
    return p_->editor;
}

void FrontScene::sendValue(const QString &idName, Float value)
{
    if (p_->editor)
        p_->editor->setUiValue(idName, CurrentTime::time(), value);
}


void FrontScene::Private::addItem(AbstractFrontItem *item)
{
    // add to scene (if not already)
    if (!item->parentItem() || item->parentItem()->scene() != gscene)
        gscene->addItem(item);

    // init
    item->setEditMode(editMode);

    // ensure unique id
    if (usedIds.contains(item->idName()))
    {
//        auto oldid = item->idName();
        item->setNewId();
//        MO_DEBUG("changed id '" << oldid << "' to '" << item->idName())
    }
    usedIds.insert(item->idName());

    // children
    auto childs = item->childFrontItems();
    for (auto c : childs)
        addItem(c);
}
/*
void FrontScene::Private::createItems(Object *root, const QPointF& pos, AbstractFrontItem *parent)
{
    qreal maxh = pos.y();
    auto localPos = pos;
    for (Parameter * p : root->params()->parameters())
    {
        if (!p->isVisibleInterface())
            continue;

        // create/add item
        auto item = new AbstractFrontItem(p, parent);
        item->setPos(localPos);
        addItem(item);

        // keep track of insert pos
        // XXX bullshit))
        localPos.rx() += item->rect().width() + 4.;
        maxh = std::max(maxh, item->rect().bottom() + 4.);
    }

    localPos.ry() = maxh;

    // recurse
    for (Object * c : root->childObjects())
        createItems(c, pos, 0);
}
*/

void FrontScene::clearInterface()
{
    // get all IDs
    QStringList ids;
    auto list = frontItems();
    for (auto i : list)
        ids << i->idName();

    clear();
    p_->usedIds.clear();
    emit itemsDeleted(ids);
    emit itemUnselected();
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

    item->setPos(snapGrid(pos));
    p_->addItem(item);
    return item;
}

AbstractFrontItem * FrontScene::createNew(FrontItemType type, const QPointF &pos)
{
    AbstractFrontItem * parent;
    QPointF p(pos);

    getLocalPos(p, &parent);

    return createNew(type, parent, p);
}

void FrontScene::removeItems(const QList<AbstractFrontItem *> &items)
{
    // get all items affected
    QList<AbstractFrontItem*> all = getAllItems(items);
    // get their IDs
    QList<QString> allIds;
    for (auto i : all)
        allIds << i->idName();

    // remove top-level items
    for (auto i : items)
        removeItem(i);

    // signal gui and scene
    emit itemsDeleted(allIds);
}

void FrontScene::groupItems(const QList<AbstractFrontItem *> &items)
{
    if (items.isEmpty())
        return;

    auto topitems = reduceToCommonParent(items);
    if (topitems.isEmpty())
        return;

    auto parent = dynamic_cast<AbstractFrontItem*>( topitems.at(0)->parentItem() );

    auto group = createNew(FIT_GROUP, parent);

    QRectF r;
    for (AbstractFrontItem * i : items)
    {
        QRectF b = i->mapToScene( i->boundingRect() ).boundingRect();
        r |= b;
        // !! not final position, only temporary store
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

QRectF FrontScene::snapGrid(const QRectF & rect) const
{
    QRectF r(rect);
    r.setLeft(MATH::quant(r.left(), 8.));
    r.setRight(MATH::quant(r.right(), 8.));
    return r;
}

QPointF FrontScene::snapGrid(const QPointF & po) const
{
    return QPointF(
                MATH::quant(po.x(), 8.),
                MATH::quant(po.y(), 8.));
}

void FrontScene::getLocalPos(QPointF& inout, AbstractFrontItem** item) const
{
    auto qi = itemAt(inout, QTransform());
    if (auto ai = dynamic_cast<AbstractFrontItem*>(qi))
    {
        *item = ai;
        inout = ai->mapFromScene(inout);
        return;
    }

    *item = 0;
}

AbstractFrontItem * FrontScene::itemForId(const QString &id, const AbstractFrontItem * ignore) const
{
    auto list = frontItems();
    for (auto i : list)
        if (i->idName() == id && i != ignore)
            return i;
    return 0;
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

/** @todo This might fail for some edge-cases, not completely tested yet. */
QList<AbstractFrontItem*> FrontScene::reduceToCommonParent(const QList<AbstractFrontItem*>& l)
{
    // find and count common parents
    QMap<QGraphicsItem*, int> map;
    for (AbstractFrontItem * i : l)
    {
        auto j = map.find(i->parentItem());
        if (j == map.end())
            map.insert(i->parentItem(), 1);
        else
            j.value()++;
    }

//    MO_DEBUG("P---");
//    for (auto i = map.begin(); i != map.end(); ++i)
//        MO_DEBUG(i.key() << " " << i.value());

    // all have the same parent? (or list was empty)
    if (map.size() <= 1)
        return l;

    // find the topmost parent
    QGraphicsItem * par = 0;
    int count = 100000000;
    for (auto i = map.begin(); i != map.end(); ++i)
    {
        // count upwards
        QGraphicsItem * p = i.key();
        if (p == 0) { par = 0; break; } // scene is the topmost really
        int c = 0;
        while (p->parentItem()) { ++c; p = p->parentItem(); }
        // find the least
        if (c < count)
        {
            par = p;
            count = c;
        }
    }

    QList<AbstractFrontItem*> list;
    for (AbstractFrontItem * i : l)
        if (i->parentItem() == par)
            list << i;

//    MO_DEBUG("---");
//    for (auto i : list)
//        MO_DEBUG(i->name());

    return list;
}

QList<AbstractFrontItem*> FrontScene::getAllItems(const QList<AbstractFrontItem*>& items)
{
    QSet<AbstractFrontItem*> set;
    for (auto i : items)
        Private::getAllItems(i, set);
    return set.toList();
}

void FrontScene::Private::getAllItems(AbstractFrontItem *item, QSet<AbstractFrontItem *> &set)
{
    set << item;

    auto list = item->childFrontItems();
    for (auto i : list)
        getAllItems(i, set);
}

QPointF FrontScene::getTopLeftPosition(const QList<AbstractFrontItem*>& list)
{
    if (list.isEmpty())
        return QPointF();
    QPointF p = list.at(0)->pos();
    for (auto i : list)
    {
        p.rx() = std::min(p.x(), i->pos().x());
        p.ry() = std::min(p.y(), i->pos().y());
    }
    return p;
}

void FrontScene::addItems(const QList<AbstractFrontItem*>& list, AbstractFrontItem * parent)
{
    if (list.isEmpty())
        return;

    // find parent to insert
    while (parent && !parent->canHaveChildren())
        parent = parent->parentFrontItem();

    QPointF tl = getTopLeftPosition(list);
    parent->setPos(parent->pos() - tl);

    for (auto i : list)
    {
        if (parent)
            i->setParentItem(parent);
        p_->addItem(i);
    }
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

    p_->createEditActions();
}


#if 0
QList<QAction*> FrontScene::createDefaultActions()
{
    QList<QAction*> list;
    QAction * a;

    list.push_back( a = new QAction(tr("Edit"), this) );
    a->setCheckable(true);
    a->setChecked(isEditMode());
    a->setShortcut(Qt::ALT + Qt::Key_E);
    connect(a, &QAction::triggered, [=]() { setEditMode(!isEditMode()); });

    list.push_back( a = new QAction(tr("New group"), this) );
    a->setShortcut(Qt::CTRL + Qt::Key_1);
    connect(a, &QAction::triggered, [=]() { if (isEditMode()) createNew(FIT_GROUP, cursorPos()); });

    list.push_back( a = new QAction(tr("New float control"), this) );
    a->setShortcut(Qt::CTRL + Qt::Key_2);
    connect(a, &QAction::triggered, [=]() { if (isEditMode()) createNew(FIT_FLOAT, cursorPos()); });

    list.push_back( a = new QAction(tr("Group selected items"), this) );
    a->setShortcut(Qt::ALT + Qt::Key_G);
    connect(a, &QAction::triggered, [=]() { if (isEditMode()) groupItems(selectedFrontItems()); });

    return list;
}
#endif

QMenu* FrontScene::Private::createNewItemMenu(const QPointF& insertPos)
{
    auto m = new QMenu(tr("create new"));
    QAction * a;

    a = m->addAction(tr("Group"));
    a->setShortcut(Qt::ALT + Qt::Key_1);
    a->setStatusTip(tr("Creates a new group item"));
    connect(a, &QAction::triggered, [=]() { gscene->createNew(FIT_GROUP, insertPos); });

    a = m->addAction(tr("Float value"));
    a->setShortcut(Qt::ALT + Qt::Key_2);
    a->setStatusTip(tr("Creates a new float controller"));
    connect(a, &QAction::triggered, [=]() { gscene->createNew(FIT_FLOAT, insertPos); });

    return m;
}

void FrontScene::Private::createEditActions()
{
    for (QAction * a : editActions)
    {
        if (a->menu() && !a->menu()->parent() && !a->menu()->parentWidget())
            delete a->menu();
        a->deleteLater();
    }
    editActions.clear();

    auto sel = gscene->selectedFrontItems();
    auto seltop = gscene->reduceToCommonParent(sel);

    QAction * a;

    // clicked into empty space
    if (sel.size() == 0)
    {
        // NEW
        editActions.addMenu(createNewItemMenu(QPointF(0,0)), gscene);
        editActions.addSeparator(gscene);

        // PASTE
        if (gscene->isItemsInClipboard())
        {
            editActions.append( a = new QAction(tr("paste"), gscene) );
            a->setShortcut(Qt::CTRL + Qt::Key_V);
            connect(a, &QAction::triggered, [=]()
            {
                auto list = gscene->getItemsFromClipboard();
                gscene->addItems(list);
            });
        }
    }

    // single item actions
    if (seltop.size() == 1)
    {
        AbstractFrontItem * item = seltop[0];

        editActions.addTitle(QString("\"%1\"").arg(item->name()), gscene);

        // NEW
        editActions.addMenu(createNewItemMenu(item->mapToScene(0,0)), gscene);

        editActions.addSeparator(gscene);

        // GROUP
        editActions.append( a = new QAction(tr("Group"), gscene) );
        a->setShortcut(Qt::ALT + Qt::Key_G);
        connect(a, &QAction::triggered, [=]()
        {
            gscene->groupItems(seltop);
        });

        editActions.addSeparator(gscene);

        // COPY
        editActions.append( a = new QAction(tr("Copy"), gscene) );
        a->setShortcut(Qt::CTRL + Qt::Key_C);
        connect(a, &QAction::triggered, [=]()
        {
            gscene->copyToClipboard(seltop);
        });

        // CUT
        editActions.append( a = new QAction(tr("Cut"), gscene) );
        a->setShortcut(Qt::CTRL + Qt::Key_X);
        connect(a, &QAction::triggered, [=]()
        {
            if (!gscene->copyToClipboard(seltop))
                return;
            gscene->removeItems(QList<AbstractFrontItem*>() << item);
            delete item;
        });

        // DELETE
        editActions.append( a = new QAction(tr("Delete"), gscene) );
        a->setShortcut(Qt::Key_Delete);
        connect(a, &QAction::triggered, [=]()
        {
            gscene->removeItems(QList<AbstractFrontItem*>() << item);
            delete item;
        });

        // PASTE
        if (gscene->isItemsInClipboard())
        {
            editActions.append( a = new QAction(tr("Paste into"), gscene) );
            a->setShortcut(Qt::CTRL + Qt::Key_V);
            connect(a, &QAction::triggered, [=]()
            {
                auto list = gscene->getItemsFromClipboard();
                gscene->addItems(list, item);
            });
        }
    }

    // multi-item actions
    if (seltop.size() > 1)
    {
        editActions.addTitle(tr("%1 items").arg(seltop.size()), gscene);

        // GROUP
        editActions.append( a = new QAction(tr("Group"), gscene) );
        a->setShortcut(Qt::ALT + Qt::Key_G);
        connect(a, &QAction::triggered, [=]()
        {
            gscene->groupItems(seltop);
        });


        editActions.addSeparator(gscene);


        // COPY
        editActions.append( a = new QAction(tr("Copy"), gscene) );
        a->setShortcut(Qt::CTRL + Qt::Key_C);
        connect(a, &QAction::triggered, [=]()
        {
            gscene->copyToClipboard(seltop);
        });

        // CUT
        editActions.append( a = new QAction(tr("Cut"), gscene) );
        a->setShortcut(Qt::CTRL + Qt::Key_X);
        connect(a, &QAction::triggered, [=]()
        {
            if (!gscene->copyToClipboard(seltop))
                return;
            gscene->removeItems(seltop);
        });

        // DELETE
        editActions.append( a = new QAction(tr("Delete"), gscene) );
        a->setShortcut(Qt::Key_Delete);
        connect(a, &QAction::triggered, [=]()
        {
            emit gscene->itemUnselected();
            gscene->removeItems(seltop);
        });

    }

    emit gscene->actionsChanged(editActions);
}



// ----------------------------- clipboard ---------------------------------

const QString FrontScene::FrontItemMimeType = "matrixoptimizer/frontitems-xml";

bool FrontScene::isItemsInClipboard() const
{
    return application->clipboard()->mimeData()->hasFormat(FrontItemMimeType);
}

bool FrontScene::copyToClipboard(const QList<AbstractFrontItem *> & list) const
{
    MO_ASSERT(list.size() == reduceToCommonParent(list).size(),
              "Only top-level items allowed in FrontScene::copyToClipboard!");

    // serialize the items
    IO::XmlStream xml;
    xml.startWriting();
    try
    {
        serialize(xml, list);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(0, tr("item clipboard"),
                              tr("Failed to copy the items to the clipboard.\n%1")
                              .arg(e.what()));
        xml.stopWriting();
        return false;
    }

    xml.stopWriting();

    // create the mimedata
    auto data = new QMimeData;
    data->setData(FrontItemMimeType, xml.data().toUtf8());
    // put into clipboard
    application->clipboard()->setMimeData(data);

    return true;
}

QList<AbstractFrontItem*> FrontScene::getItemsFromClipboard() const
{
    QList<AbstractFrontItem*> list;

    auto data = application->clipboard()->mimeData();
    if (!data->hasFormat(FrontItemMimeType))
        return list;

    // deserialize
    IO::XmlStream xml;
    xml.setData(QString::fromUtf8(data->data(FrontItemMimeType)));
    xml.startReading();
    try
    {
        while (xml.nextSubSection())
        {
            if (xml.section() == "user-interface")
                deserialize(xml, list);
            xml.leaveSection();
        }
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(0, tr("item clipboard"),
                              tr("Failed to paste the items from the clipboard.\n%1")
                              .arg(e.what()));
    }

    return list;
}


namespace { const qreal FAR = 10000000.; }

void FrontScene::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    if (isEditMode())
    {
        //update(viewsRect());
        update();
    }

    QGraphicsScene::mouseMoveEvent(e);
}

void FrontScene::drawForeground(QPainter *p, const QRectF &)
{
    if (isEditMode())
    {
        auto cpos = cursorPos();

        p->setPen(QPen(QColor(255,255,255,100)));
        p->drawLine(-FAR, cpos.y(), FAR, cpos.y());
        p->drawLine(cpos.x(), -FAR, cpos.x(), FAR);
    }
}

} // namespace GUI
} // namespace MO
