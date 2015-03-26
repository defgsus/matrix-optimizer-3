/** @file scopeitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.03.2015</p>
*/

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsScene>

#include "scopeitem.h"
#include "gui/painter/valuecurve.h"
#include "io/error.h"

namespace MO {
namespace GUI {

ScopeItem::ScopeItem(QGraphicsItem * p)
    : AbstractGuiItem   (p)
    , min_              (0.)
    , max_              (100.)
    , color_            (QColor(100,150,100))
    , colorBack_        (QColor(30,30,30))
    , draw_             (new PAINTER::ValueCurve())
{
}

ScopeItem::~ScopeItem()
{
    delete draw_;
}

void ScopeItem::setRange(Float mi, Float ma)
{
    min_ = mi;
    max_ = ma;
    update();
}



void ScopeItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    // background
    p->setPen(Qt::NoPen);
    p->setBrush(QBrush(colorBack_));
    QRectF r = rect();

    p->drawRect(r);

    //draw_->setC
}


} // namespace GUI
} // namespace MO
