/** @file abstractfrontitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QPainter>
#include <QStaticText>

#include "abstractfrontitem.h"
#include "gui/util/frontscene.h"
#include "types/properties.h"
#include "object/param/parameter.h"
#include "io/xmlstream.h"
#include "io/log.h"
#include "io/error.h"


namespace MO {
namespace GUI {

QMap<QString, AbstractFrontItem*> AbstractFrontItem::p_reg_items_;

AbstractFrontItem::AbstractFrontItem(QGraphicsItem* parent)
    : QGraphicsItem     (parent)
    , p_props_          (new Properties)
    , p_statictext_name_(0)
    , p_editMode_       (false)
{
    setFlag(ItemIsFocusable, true);
    setFlag(ItemSendsGeometryChanges, true);

    initProperty("position", QPoint(0, 0));
    initProperty("size", QSize(64, 64));
    initProperty("rounded-size", QSize(0, 0));

    initProperty("label-text", QString());

    initProperty("padding", 5);
    initProperty("border", 0);

    initProperty("label-visible", true);
    initProperty("label-outside", true);
    initProperty("label-align", Properties::Alignment(Properties::A_TOP | Properties::A_HCENTER));
    initProperty("label-margin", 0);

    initProperty("background-color", QColor(40,40,50));
    initProperty("border-color", QColor(200,200,220));

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

    return QString("Item[%1]").arg(p_props_->get("label-text").toString());
}

// -------------------------------- io -----------------------------------------

void AbstractFrontItem::serialize(IO::XmlStream& io) const
{
    io.newSection("interface-item");

        io.write("version", 1);
        io.write("class", className());

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
        Q_UNUSED(ver);

        QString classn = io.expectString("class");

        auto item = factory(classn);
        if (!item)
            return 0;
        /** @todo auto_ptr or something to avoid leaks on read-errors */

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
                child->setParentItem(item);
            }

            io.leaveSection();
        }

    return item;
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
    setFlag(ItemIsSelectable, e);
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
    return innerRect().adjusted(-pad, -pad, pad, pad);
}

QRectF AbstractFrontItem::boundingRect() const
{
    qreal pad = p_props_->get("padding").toInt()
                + p_props_->get("border").toInt();
    return innerRect().adjusted(-pad, -pad, pad, pad)
            | p_labelRect_;
}

QColor AbstractFrontItem::borderColor() const
{
    QColor c = p_props_->get("border-color").value<QColor>();
    if (isSelected())
       c = c.lighter();
    return c;
}

QColor AbstractFrontItem::backgroundColor() const
{
    QColor c = p_props_->get("background-color").value<QColor>();
    if (isSelected())
       c = c.lighter();
    return c;
}

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
    p->setBrush(QBrush(backgroundColor()));
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
