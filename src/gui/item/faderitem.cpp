/** @file faderitem.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QPainter>

#include "faderitem.h"

namespace MO {
namespace GUI {

FaderItem::FaderItem(QGraphicsItem * p)
    : AbstractGuiItem     (p)
{
}


void FaderItem::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    p->setBrush(QBrush(Qt::yellow));
    p->setPen(Qt::NoPen);

    p->drawRect(rect());
}


} // namespace GUI
} // namespace MO
