/** @file sequenceoverpaint.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequenceoverpaint.h"

namespace MO {
namespace GUI {
namespace PAINTER {

SequenceOverpaint::SequenceOverpaint(QObject *parent) :
    QObject     (parent),
    sequence_   (0)
{
    penLoop_ = QPen(QColor(255,255,100,100));
    brushOutside_ = QBrush(QColor(0,0,0,50));
}

void SequenceOverpaint::paint(QPainter & p)
{
    paint(p, p.window());
}

void SequenceOverpaint::paint(QPainter &p, const QRect &rect)
{
    if (!sequence_)
        return;

    p.setPen(Qt::NoPen);
    p.setBrush(brushOutside_);
}


} // namespace PAINTER
} // namespace GUI
} // namespace MO
