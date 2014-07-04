/** @file sequencewidget.cpp

    @brief Widget for display MO::Sequence in MO::GUI::TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#include "sequencewidget.h"
#include "io/log.h"

namespace MO {
namespace GUI {

SequenceWidget::SequenceWidget(Track * track, Sequence * seq, QWidget *parent) :
    QWidget     (parent),
    track_      (track),
    sequence_   (seq)
{
    MO_DEBUG_GUI("SequenceWidget::SequenceWidget(" << track << ", " << seq << ", " << parent << ")");

    setAutoFillBackground(true);

    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(80,120+rand()%40,80));
    setPalette(p);

}


void SequenceWidget::paintEvent(QPaintEvent * e)
{
    QWidget::paintEvent(e);
}

} // namespace GUI
} // namespace MO
