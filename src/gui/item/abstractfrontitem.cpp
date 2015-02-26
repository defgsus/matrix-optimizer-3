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

//#include "faderitem.h"

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
        //p_props_->set("parameter-id", p->idName());
        p_props_->set("name", p->name());
    }
    else
        p_props_->set("name", QString("new"));

    p_props_->set("padding", 5);
    p_props_->set("border", 3);

    p_props_->set("label-visible", true);
    p_props_->set("label-align", Properties::Alignment(Properties::A_TOP | Properties::A_HCENTER));
    p_props_->set("label-margin", 0);

    p_props_->set("background-color", QColor(40,40,50));
    p_props_->set("border-color", QColor(200,200,220));

    p_props_->set("size", QSize(64, 64));
    p_props_->set("rounded-size", QSize(6, 6));

    // merge with actual Parameter's properties
    if (p_param_)
        p_props_->merge(p_param_->interfaceProperties());

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
    // tell parameter (which does the actual disk io)
    if (p_param_)
        p_param_->setInterfaceProperties(*p_props_);

    // keep track of size changes
    if (p_oldSize_ != innerRect().size())
        prepareGeometryChange();
    p_oldSize_ = innerRect().size();

    update();
}

// ---------------------------------- layout -----------------------------------


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
    return innerRect().adjusted(-pad, -pad, pad, pad);
}

void AbstractFrontItem::paint(QPainter * p, const QStyleOptionGraphicsItem * , QWidget * )
{
    // set colors/pens
    QPen bpen( p_props_->get("border-color").value<QColor>() );
    p->setBrush(QBrush(p_props_->get("background-color").value<QColor>()));
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

        const QString name = p_props_->get("name").toString();
        if (p_statictext_name_->text() != name)
            p_statictext_name_->setText(name);

        // alignment
        QRectF lr(rec);
        lr.setSize( p_statictext_name_->size() );
        lr = Properties::align(lr, rec, p_props_->get("label-align").toInt(),
                                        p_props_->get("label-margin").toInt());
        // draw
        p->setPen(bpen);
        p->drawStaticText(lr.topLeft(), *p_statictext_name_);
    }
}


} // namespace GUI
} // namespace MO
