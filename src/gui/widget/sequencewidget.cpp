/** @file sequencewidget.cpp

    @brief Widget for display MO::Sequence in MO::GUI::TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#include "sequencewidget.h"

namespace MO {
namespace GUI {

SequenceWidget::SequenceWidget(Sequence * seq, QWidget *parent) :
    QWidget(parent),
    sequence_   (seq)
{
    setAutoFillBackground(true);

    if (seq)
        setSequence(seq);
}


void SequenceWidget::setSequence(Sequence * seq)
{
    sequence_  = seq;

    QPalette p(palette());
    p.setColor(QPalette::Background, QColor(80,120,80));
    setPalette(p);
}




} // namespace GUI
} // namespace MO
