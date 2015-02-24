/** @file abstractfrontitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QPainter>
#include <QStaticText>

#include "abstractfrontitem.h"
#include "types/properties.h"
#include "object/param/parameter.h"
#include "io/log.h"

#include "faderitem.h"

namespace MO {
namespace GUI {


AbstractFrontItem::AbstractFrontItem(Parameter* p, QGraphicsItem* parent)
    : QGraphicsItem     (parent)
    , p_props_          (new Properties)
    , p_param_          (p)
    , p_statictext_name_(0)
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsFocusable, true);
    setFlag(ItemSendsGeometryChanges, true);

    if (p)
    {
        p_props_->set("parameter-id", p->idName());
        p_props_->set("name", p->name());
    }
    else
        p_props_->set("name", "new");

    p_props_->set("padding", 5);
    p_props_->set("show-label", true);
    p_props_->set("border", 2);

    p_props_->set("border-color", QColor(200,200,220));
    p_props_->set("background-color", QColor(40,40,50));

    p_update_from_properties_();
/*
    auto f = new FaderItem(this);
    f->setRect(innerRect());
*/
}

AbstractFrontItem::~AbstractFrontItem()
{
    delete p_props_;
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

void AbstractFrontItem::p_update_from_properties_()
{
    update();
}

// ---------------------------------- layout -----------------------------------


QRectF AbstractFrontItem::innerRect() const
{
    return QRectF(0,0, p_props_->get("width", 64).toInt(),
                       p_props_->get("height", 64).toInt());
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
    return innerRect().adjusted(-pad, -pad, pad, pad);
}

void AbstractFrontItem::paint(QPainter * p, const QStyleOptionGraphicsItem * , QWidget * )
{
    // set colors/pens
    p->setBrush(QBrush(p_props_->get("background-color").value<QColor>()));
    int bor = p_props_->get("border").toInt();
    if (bor > 0)
    {
        QPen pen( p_props_->get("border-color").value<QColor>() );
        pen.setWidth( bor );
        p->setPen(pen);
    }
    else
        p->setPen(Qt::NoPen);

    auto rec = rect();
    p->drawRoundedRect(rec, 10., 10.);

    // label
    if (p_props_->get("show-label").toBool())
    {
        if (!p_statictext_name_)
            p_statictext_name_ = new QStaticText();

        const QString name = p_props_->get("name").toString();
        if (p_statictext_name_->text() != name)
            p_statictext_name_->setText(name);

        p->drawStaticText(rec.topLeft(), *p_statictext_name_);
    }
}


} // namespace GUI
} // namespace MO
