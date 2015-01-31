/** @file abstractfrontitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QPainter>
#include <QStaticText>

#include "abstractfrontitem.h"
#include "faderitem.h"
#include "object/param/parameter.h"
#include "io/log.h"

namespace MO {
namespace GUI {


AbstractFrontItem::AbstractFrontItem(Parameter* p, QGraphicsItem* parent)
    : QGraphicsItem     (parent)
    , p_param_          (p)
    , p_statictext_name_(0)
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsFocusable, true);
    setFlag(ItemSendsGeometryChanges, true);

    if (p)
    {
        setProperty("parameter-id", p->idName());
        setProperty("name", p->name());
    }
    else
        setProperty("name", "new");

    setProperty("padding", 5);
    setProperty("show-label", true);
    setProperty("border", 2);

    setProperty("border-color", QColor(200,200,220));
    setProperty("background-color", QColor(40,40,50));

    p_update_from_properties_();

    auto f = new FaderItem(this);
    f->setRect(innerRect());
    //scene()->addItem(f);
}


// -------------------------------- properties ---------------------------------

QVariant AbstractFrontItem::getProperty(const QString &id) const
{
    auto i = p_props_.find(id);
#ifdef MO_DO_DEBUG
    if (i == p_props_.end())
        MO_DEBUG("AbstractFrontItem: unknown property '" << id << "' requested.")
#endif
    return i == p_props_.end() ? QVariant() : i.value();
}

QVariant AbstractFrontItem::getProperty(const QString &id, const QVariant& def) const
{
    auto i = p_props_.find(id);
    return i == p_props_.end() ? def : i.value();
}

void AbstractFrontItem::setProperty(const QString &id, const QVariant & v)
{
    p_props_.insert(id, v);
}

void AbstractFrontItem::p_update_from_properties_()
{

}

// ---------------------------------- layout -----------------------------------


QRectF AbstractFrontItem::innerRect() const
{
    return QRectF(0,0, getProperty("width", 64).toInt(),
                       getProperty("height", 64).toInt());
}

QRectF AbstractFrontItem::outerRect() const
{
    int pad = getProperty("padding").toInt();
    return innerRect().adjusted(-pad, -pad, pad, pad);
}

QRectF AbstractFrontItem::boundingRect() const
{
    qreal pad = getProperty("padding").toInt()
                + getProperty("border").toInt();
    return innerRect().adjusted(-pad, -pad, pad, pad);
}

void AbstractFrontItem::paint(QPainter * p, const QStyleOptionGraphicsItem * , QWidget * )
{
    // set colors/pens
    p->setBrush(QBrush(getProperty("background-color").value<QColor>()));
    int bor = getProperty("border").toInt();
    if (bor > 0)
    {
        QPen pen( getProperty("border-color").value<QColor>() );
        pen.setWidth( bor );
        p->setPen(pen);
    }
    else
        p->setPen(Qt::NoPen);

    auto rec = outerRect();
    p->drawRoundedRect(rec, 10., 10.);

    // label
    if (getProperty("show-label").toBool())
    {
        if (!p_statictext_name_)
            p_statictext_name_ = new QStaticText();

        const QString name = getProperty("name").toString();
        if (p_statictext_name_->text() != name)
            p_statictext_name_->setText(name);

        p->drawStaticText(rec.topLeft(), *p_statictext_name_);
    }
}


} // namespace GUI
} // namespace MO
