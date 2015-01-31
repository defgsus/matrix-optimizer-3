/** @file abstractfrontitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QPainter>
#include <QStaticText>

#include "abstractfrontitem.h"
#include "object/param/parameter.h"

namespace MO {
namespace GUI {


AbstractFrontItem::AbstractFrontItem(FrontItemType type, Parameter* p, QGraphicsItem* parent)
    : QGraphicsItem     (parent)
    , p_type_           (type)
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
}


// -------------------------------- properties ---------------------------------

QVariant AbstractFrontItem::getProperty(const QString &id) const
{
    auto i = p_props_.find(id);
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


QRectF AbstractFrontItem::rect() const
{
    return QRectF(0,0, getProperty("width", 64).toInt(),
                       getProperty("height", 64).toInt());
}


QRectF AbstractFrontItem::boundingRect() const
{
    const auto r = rect();
    return r;
    /*const qreal pw = ObjectGraphSettings::penOutlineWidth();
    return r.adjusted(-pw,-pw, pw, pw);*/
}

void AbstractFrontItem::paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *)
{
    // set colors/pens
    p->setBrush(QBrush(getProperty("background-color").value<QColor>()));
    int bor = getProperty("border", 2).toInt();
    if (bor > 0)
    {
        QPen pen( getProperty("border-color").value<QColor>() );
        pen.setWidth( bor );
        p->setPen(pen);
    }
    else
        p->setPen(Qt::NoPen);

    p->drawRoundedRect(rect(), 10., 10.);

    // label
    if (getProperty("show-label").toBool())
    {
        if (!p_statictext_name_)
            p_statictext_name_ = new QStaticText();

        const QString name = getProperty("name").toString();
        if (p_statictext_name_->text() != name)
            p_statictext_name_->setText(name);

        p->drawStaticText(0, 0, *p_statictext_name_);
    }
}


} // namespace GUI
} // namespace MO
