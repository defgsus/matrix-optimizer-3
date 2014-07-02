/** @file sequenceoverpaint.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QPainter>

#include "sequenceoverpaint.h"
#include "object/sequence.h"


namespace MO {
namespace GUI {
namespace PAINTER {

SequenceOverpaint::SequenceOverpaint(QObject *parent) :
    QObject     (parent),
    sequence_   (0)
{
    penLoop_ = QPen(QColor(255,255,100,100));
    brushOutside_ = QBrush(QColor(0,0,0,150));
}

void SequenceOverpaint::paint(QPainter & p)
{
    paint(p, p.window());
}

void SequenceOverpaint::paint(QPainter &p, const QRect &rect)
{
    // -- outside --

    if (sequence_)
    {
        int left = viewspace_.mapXFrom(sequence_->start()) * p.window().width(),
            right = viewspace_.mapXFrom(sequence_->end()) * p.window().width();

        if (left > rect.left() || right < rect.right())
        {
            p.setPen(Qt::NoPen);
            p.setBrush(brushOutside_);

            if (left > rect.left())
                p.drawRect(rect.left(), rect.top(),
                           left - rect.left(), rect.height());

            if (right < rect.right())
                p.drawRect(right, rect.top(),
                           rect.right() - right, rect.height());
        }
    }
}


} // namespace PAINTER
} // namespace GUI
} // namespace MO
