/** @file sequenceoverpaint.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QPainter>

#include "sequenceoverpaint.h"
#include "object/sequence.h"
#include "object/scene.h"

namespace MO {
namespace GUI {
namespace PAINTER {

SequenceOverpaint::SequenceOverpaint(QObject *parent) :
    QObject     (parent),
    sequence_   (0)
{
    penLoop_ = QPen(QColor(255,255,100,100));
    penLoop_.setStyle(Qt::DashLine);
    penLoop_.setWidth(2);
    brushOutside_ = QBrush(QColor(0,0,0,150));
}

QRect SequenceOverpaint::playBarRect(const QRect& widget) const
{
    if (const Scene * scene = sequence_->sceneObject())
    {
        int x = viewspace_.mapXFrom(scene->sceneTime()) * widget.width();
        return QRect(x,0,x,widget.height());
    }
    return QRect();
}

void SequenceOverpaint::paint(QPainter & p)
{
    paint(p, p.window());
}

void SequenceOverpaint::paint(QPainter &p, const QRect &rect)
{
    if (sequence_)
    {
        // -- inside / outside indicator --

        int left = viewspace_.mapXFrom(0.0) * p.window().width(),
            // Sequences in Clips are unbounded
            right = sequence_->parentClip() ?
                    rect.right()
                  : viewspace_.mapXFrom(sequence_->length() / sequence_->speed()) * p.window().width();

        if (left > rect.left() || right < rect.right())
        {
            p.setPen(Qt::NoPen);
            p.setBrush(brushOutside_);

            if (left > rect.left())
                p.drawRect(rect.left(), rect.top(),
                           left - rect.left(), rect.height());

            if (right <= rect.right())
                p.drawRect(right, rect.top(),
                           rect.right() - right + 1, rect.height());
        }

        // -- loop indicators --

        if (sequence_->looping())
        {
            p.setPen(penLoop_);

            Double t = (sequence_->loopStart() - sequence_->timeOffset()) / sequence_->speed();
            int x = viewspace_.mapXFrom(t) * p.window().width();
            p.drawLine(x, rect.top(), x, rect.bottom());

            t = (sequence_->loopStart() + sequence_->loopLength() - sequence_->timeOffset())
                    / sequence_->speed();
            x = viewspace_.mapXFrom(t) * p.window().width();
            p.drawLine(x, rect.top(), x, rect.bottom());
        }
    }
}


} // namespace PAINTER
} // namespace GUI
} // namespace MO
