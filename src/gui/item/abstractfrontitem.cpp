/** @file abstractfrontitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <memory>

#include <QPainter>
#include <QStaticText>
#include <QDrag>
#include <QMimeData>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneDragDropEvent>

#include "abstractfrontitem.h"
#include "gui/util/frontscene.h"
#include "types/properties.h"
#include "object/param/parameter.h"
#include "io/application.h"
#include "io/xmlstream.h"
#include "io/log.h"
#include "io/error.h"


namespace MO {
namespace GUI {

// ---------------------------- FrontItemMimeData ------------------------------

const QString FrontItemMimeData::ItemIdMimeType = "matrixoptimizer/ui-item-id";
const QString FrontItemMimeData::ItemPtrMimeType = "matrixoptimizer/ui-item-ptr";
const QString FrontItemMimeData::AppPtrMimeType = "matrixoptimizer/app-ptr";

FrontItemMimeData * FrontItemMimeData::frontItemMimeData(QMimeData* data)
{
    return data->hasFormat(ItemIdMimeType) ?
        static_cast<FrontItemMimeData*>(data) : 0;
}

const FrontItemMimeData * FrontItemMimeData::frontItemMimeData(const QMimeData* data)
{
    return data->hasFormat(ItemIdMimeType) ?
        static_cast<const FrontItemMimeData*>(data) : 0;
}

void FrontItemMimeData::setItem(AbstractFrontItem* item)
{
    setData(ItemIdMimeType, item->idName().toUtf8());
    setData(ItemPtrMimeType,
            QByteArray::fromRawData(reinterpret_cast<const char*>(item), sizeof(item)) );
    setData(AppPtrMimeType,
            QByteArray::fromRawData(reinterpret_cast<const char*>(application()), sizeof(application())) );
}

QString FrontItemMimeData::getItemId() const
{
    if (!hasFormat(ItemIdMimeType))
        return QString();
    return QString::fromUtf8(data(ItemIdMimeType));
}

int FrontItemMimeData::modulatorType() const
{
    if (!isSameApplicationInstance())
        return 0;

    auto i = getItem();
    return i ? i->modulatorType() : 0;
}

AbstractFrontItem * FrontItemMimeData::getItem() const
{
    if (!hasFormat(ItemPtrMimeType))
        return 0;
    const char * d = data(ItemPtrMimeType).constData();
    auto item = reinterpret_cast<const AbstractFrontItem*>(d);
    return const_cast<AbstractFrontItem*>(item);
}

bool FrontItemMimeData::isSameApplicationInstance() const
{
    if (!hasFormat(AppPtrMimeType))
        return true;
    const char * d = data(AppPtrMimeType).constData();
    auto app = reinterpret_cast<const Application*>(d);
    return app == application();
}





// -------------------------------- AbstractFrontItem --------------------------


QMap<QString, AbstractFrontItem*> AbstractFrontItem::p_reg_items_;
int AbstractFrontItem::p_id_count_ = 0;

AbstractFrontItem::AbstractFrontItem(QGraphicsItem* parent)
    : QGraphicsItem     (parent)
    , p_props_          (new Properties)
    , p_statictext_name_(0)
    , p_headerHeight_   (0)
    , p_editMode_       (false)
    , p_canHaveChildren_(false)
    , p_startDrag_      (false)
{
    setNewId();

    setFlag(ItemIsFocusable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setAcceptDrops(true);

    initProperty("position", QPoint(0, 0));
    initProperty("size", QSize(64, 64));
    initProperty("rounded-size", QSize(0, 0));

    initProperty("padding", 5);

    initProperty("border", 0);
    initProperty("border-color", QColor(0xa0, 0xa0, 0xa0));

    initProperty("background-color", QColor(0x20, 0x20, 0x20));
    initProperty("background-visible", true);

    initProperty("label-text", QString());
    initProperty("label-visible", true);
    initProperty("label-outside", true);
    initProperty("label-align", Properties::Alignment(Properties::A_TOP | Properties::A_HCENTER));
    initProperty("label-margin", 0);


    // pull in Parameter's properties
//    if (p_param_)
//        p_props_->unify(p_param_->interfaceProperties());


    //p_update_from_properties_();

/*
    auto f = new FaderItem(this);
    f->setRect(innerRect());
*/
}

AbstractFrontItem::~AbstractFrontItem()
{
    delete p_props_;
}

// ------------------------ factory --------------------------------------------

bool AbstractFrontItem::registerFrontItem(AbstractFrontItem * i)
{
    if (p_reg_items_.contains(i->className()))
    {
        MO_WARNING("AbstractFrontItem::registerFrontItem('"
                   << i->className() << "'): duplicate call!");
        delete i;
        return false;
    }

    p_reg_items_.insert(i->className(), i);

    return true;
}

AbstractFrontItem * AbstractFrontItem::factory(const QString &className)
{
    auto i = p_reg_items_.find(className);
    if (i == p_reg_items_.end())
    {
        MO_WARNING("Request for unknown FrontItem class '" << className << "'");
        return 0;
    }

    return i.value()->cloneClass();
}

// ------------------------ getter ---------------------------------------------

QString AbstractFrontItem::name() const
{
    //if (p_param_)
    //    return "-> " + p_param_->infoName();

    return QString("%1[%2]")
            .arg(className())
            .arg(p_props_->get("label-text").toString());
}

// -------------------------------- io -----------------------------------------

void AbstractFrontItem::serialize(IO::XmlStream& io) const
{
    io.newSection("interface-item");

        io.write("version", 2);
        io.write("class", className());
        // v2
        io.write("id", idName());

        p_props_->serialize(io);

        // write children
        auto childs = childFrontItems();
        for (auto c : childs)
            c->serialize(io);

    io.endSection();
}

AbstractFrontItem * AbstractFrontItem::deserialize(IO::XmlStream & io)
{
    io.verifySection("interface-item");

    const int ver = io.expectInt("version");

    QString classn = io.expectString("class");
    QString idn;
    if (ver >= 2)
        idn = io.expectString("id");

    // create the item
    auto item = factory(classn);
    if (!item)
        return 0;

    // avoid leaks on read-errors
    std::unique_ptr<AbstractFrontItem> aptr(item);

    // set id
    if (!idn.isEmpty())
        item->p_id_ = idn;

    // properties and children
    while (io.nextSubSection())
    {
        // properties
        if (io.section() == "properties")
            item->p_props_->deserialize(io);
        else
        // children
        if (io.section() == "interface-item")
        {
            auto child = deserialize(io);
            if (child)
                child->setParentItem(item);
        }

        io.leaveSection();
    }

    return aptr.release();
}

// -------------------------------- properties ---------------------------------

void AbstractFrontItem::setProperties(const Properties & p)
{
    *p_props_ = p;
    p_update_from_properties_();
}

void AbstractFrontItem::setProperty(const QString &id, const QVariant &v)
{
    p_props_->set(id, v);
    p_update_from_properties_();
}

void AbstractFrontItem::initProperty(const QString &id, const QVariant &v)
{
    p_props_->set(id, v);
    p_prop_changed_ = true;
}

void AbstractFrontItem::changeProperty(const QString &id, const QVariant &v)
{
    p_props_->change(id, v);
    p_update_from_properties_();
}

void AbstractFrontItem::p_update_from_properties_()
{
    // tell parameter (which does the actual disk io)
    // also bull XXX
    //if (p_param_)
    //    p_param_->setInterfaceProperties(*p_props_);

    // update item position
    QPointF p = p_props_->get("position").toPoint();
    if (p != pos())
        setPos(p);

    // keep track of bounding-rect changes
    if (p_oldRect_ != boundingRect())
        prepareGeometryChange();
    p_oldRect_ = boundingRect();

    p_prop_changed_ = false;

    // derived code
    onPropertiesChanged();

    update();
}

void AbstractFrontItem::setNewId()
{
    p_id_ = QString("item%1").arg(p_id_count_++);
}

// -------------------------------- tree ---------------------------------------


QList<AbstractFrontItem*> AbstractFrontItem::childFrontItems() const
{
    QList<AbstractFrontItem*> list;

    auto childs = childItems();
    for (auto c : childs)
        if (auto a = dynamic_cast<AbstractFrontItem*>(c))
            list << a;

    return list;
}

bool AbstractFrontItem::hasParent(const QGraphicsItem *parent) const
{
    auto p = parentItem();
    while (p)
    {
        if (p == parent)
            return true;
        p = p->parentItem();
    }
    return false;
}

AbstractFrontItem* AbstractFrontItem::parentFrontItem() const
{
    return dynamic_cast<AbstractFrontItem*>(parentItem());
}

// -------------------------------- editing ------------------------------------

void AbstractFrontItem::startDragging()
{
    auto drag = new QDrag(scene());
    auto data = new FrontItemMimeData();
    data->setItem(this);
    drag->setMimeData(data);
    drag->setPixmap(getPixmap(48));
    drag->exec(Qt::MoveAction);
}

QPixmap AbstractFrontItem::getPixmap(uint max_size)
{
    // if something has not been updated yet
    if (p_prop_changed_)
        p_update_from_properties_();

    // Need to use the bounding rect, which contains
    // outside labels and stuff like that
    QRectF r(boundingRect());

    // render
    QPixmap pix(r.width(), r.height());
    QPainter p(&pix);
    p.translate(-r.topLeft());
    p.fillRect(r, Qt::black);
    paint(&p, 0, 0);
    /** @todo Would be nice to paint the children as well */

    return pix.scaled(QSize(max_size, max_size), Qt::KeepAspectRatio);
}

// ---------------------------------- events -----------------------------------

void AbstractFrontItem::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    QGraphicsItem::mousePressEvent(e);

    if ((e->buttons() & Qt::LeftButton)
        && (e->modifiers() & Qt::SHIFT))
    {
        p_startDrag_ = true;
    }
}

void AbstractFrontItem::mouseMoveEvent(QGraphicsSceneMouseEvent * e)
{
    if (!p_startDrag_)
        QGraphicsItem::mouseMoveEvent(e);

    if (p_startDrag_
        && (e->pos() - e->buttonDownPos(Qt::LeftButton)).manhattanLength() > 4)
    {
        p_startDrag_ = false;
        startDragging();
    }
}

void AbstractFrontItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    p_startDrag_ = false;
    QGraphicsItem::mouseReleaseEvent(e);
}

void AbstractFrontItem::dragEnterEvent(QGraphicsSceneDragDropEvent * e)
{
    // generally ignore
    e->ignore();

    // another item?
    if (auto idata = FrontItemMimeData::frontItemMimeData(e->mimeData()))
    {
        if (!idata->isSameApplicationInstance())
            return;

        if (!canHaveChildren())
            return;

        QString id = idata->getItemId();
        // accept the event if it's not ourselves
        if (id != idName()
                // and not already a direct child
                && !childItems().contains(idata->getItem()))
            e->accept();
    }
}

void AbstractFrontItem::dropEvent(QGraphicsSceneDragDropEvent * e)
{
    // generally ignore
    e->ignore();

    // another item? then move into ourselves
    if (auto idata = FrontItemMimeData::frontItemMimeData(e->mimeData()))
    {
        if (!idata->isSameApplicationInstance())
            return;

        QString id = idata->getItemId();
        // accept the even if it's not ourselves
        if (id == idName())
            return;

        auto s = frontScene();
        if (!s)
            return;
        // get the item
        auto item = s->itemForId(id);
        // avoid invalid ancestry
        if (hasParent(item))
            return;
        // move here
        item->setParentItem(this);
        item->setPos(e->pos());
        e->accept();
    }
}


// ---------------------------------- layout -----------------------------------

FrontScene * AbstractFrontItem::frontScene() const
{
    if (auto s = qobject_cast<FrontScene*>(scene()))
        return s;
    return 0;
}

void AbstractFrontItem::setEditMode(bool e)
{
    p_editMode_ = e;
    setFlag(ItemIsMovable, e);
    setFlag(ItemIsSelectable, true);
    // derived code
    onEditModeChanged();
}

void AbstractFrontItem::setSize(const QSizeF & r)
{
    setProperty("size", r.toSize()); // let's keep it int for now
}

void AbstractFrontItem::setInsidePos(const QPointF & p)
{
    setPos(p + rect().topLeft());
}

QRectF AbstractFrontItem::innerRect() const
{
    QSize s = p_props_->get("size", QSize(64, 64)).toSize();
    return QRectF(0,0, s.width(), s.height());
}

QRectF AbstractFrontItem::rect() const
{
    int pad = p_props_->get("padding").toInt();
    return innerRect().adjusted(-pad, -pad-p_headerHeight_, pad, pad);
}

QRectF AbstractFrontItem::boundingRect() const
{
    qreal pad = p_props_->get("border").toInt();
    return rect().adjusted(-pad, -pad, pad, pad)
                    | p_labelRect_;
}

QColor AbstractFrontItem::borderColor() const
{
    QColor c = p_props_->get("border-color").value<QColor>();
    if (isSelected())// && isEditMode())
       c = c.lighter(120);
    return c;
}

QColor AbstractFrontItem::backgroundColor() const
{
    QColor c = p_props_->get("background-color").value<QColor>();
    if (isSelected())// && type() != FIT_GROUP)
       c = c.lighter(120);
    return c;
}


void AbstractFrontItem::setHeaderHeight(qreal h)
{
    if (h == p_headerHeight_)
        return;

    p_headerHeight_ = h;

    prepareGeometryChange();
    update();
}

QRectF AbstractFrontItem::headerRect() const
{
    auto r = innerRect();
    return QRectF(
                r.left(), r.top() - p_headerHeight_,
                r.width(), p_headerHeight_);
}




// -------------------- QGraphicsItem ---------------------------------

QVariant AbstractFrontItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange)
    {
        QPointF newPos = value.toPointF();
        // grid snapping
        auto scene = frontScene();
        if (scene)
            newPos = scene->snapGrid(newPos);
        // update properties
        p_props_->set("position", newPos.toPoint()); // keep it int
        return QPointF(newPos);
    }
    return QGraphicsItem::itemChange(change, value);
}

void AbstractFrontItem::paint(QPainter * p, const QStyleOptionGraphicsItem * , QWidget * )
{
    // XXX Not sure if a potential resize should be executed
    // in the paint event... no warnings in the doc at least
    if (p_prop_changed_)
        p_update_from_properties_();

    // set colors/pens
    QPen bpen( borderColor() );
    if (p_props_->get("background-visible").toBool())
        p->setBrush(QBrush(backgroundColor()));
    else
        p->setBrush(Qt::NoBrush);
    // border
    int bor = p_props_->get("border").toInt();
    if (bor > 0)
    {
        QPen pen(bpen);
        pen.setWidth( bor );
        p->setPen(pen);
    }
    else
        p->setPen(Qt::NoPen);

    // -- background and frame --
    auto rec = rect();
    auto rr = p_props_->get("rounded-size").toSize();
    p->drawRoundedRect(rec, rr.width(), rr.height());

    // -- edit frame --
    if (isSelected() && isEditMode())
    {
        p->setPen(QPen(Qt::green));
        p->setBrush(Qt::NoBrush);
        p->drawRect(boundingRect());
    }

    // -- label --
    if (p_props_->get("label-visible").toBool())
    {
        // update QStaticText
        if (!p_statictext_name_)
            p_statictext_name_ = new QStaticText();

        const QString text = p_props_->get("label-text").toString();
        if (p_statictext_name_->text() != text)
            p_statictext_name_->setText(text);

        // alignment
        p_labelRect_ = rec;
        p_labelRect_.setSize( p_statictext_name_->size() );
        p_labelRect_ = Properties::align(p_labelRect_, rec,
                                         p_props_->get("label-align").value<Properties::Alignment>(),
                                         p_props_->get("label-margin").toInt(),
                                         p_props_->get("label-outside").toBool());

        // draw
        p->setPen(bpen);
        p->drawStaticText(p_labelRect_.topLeft(), *p_statictext_name_);
    }
}


} // namespace GUI
} // namespace MO
