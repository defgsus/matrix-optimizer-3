/** @file generalsequencefloatview.cpp

    @brief Display for float sequences, except timelines

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QPaintEvent>
#include <QPainter>


#include "generalsequencefloatview.h"
#include "painter/grid.h"
#include "painter/valuecurve.h"
#include "painter/sequenceoverpaint.h"
#include "object/sequencefloat.h"

namespace MO {
namespace GUI {

class SequenceCurveData : public PAINTER::ValueCurveData
{
public:
    const SequenceFloat * sequence;
    Double value(Double time) const { return sequence->value(time); }
};



GeneralSequenceFloatView::GeneralSequenceFloatView(QWidget *parent) :
    QWidget         (parent),
    sequence_       (0),
    grid_           (new PAINTER::Grid(this)),
    curve_          (new PAINTER::ValueCurve(this)),
    curveData_      (new SequenceCurveData),
    over_           (new PAINTER::SequenceOverpaint(this)),

    brushBack_      (QColor(50,50,50))
{
}

int GeneralSequenceFloatView::gridOptions() const
{
    return grid_->options();
}

void GeneralSequenceFloatView::setGridOptions(int options)
{
    grid_->setOptions(options);
    update();
}

void GeneralSequenceFloatView::setViewSpace(const UTIL::ViewSpace & s, bool send_signal)
{
    space_ = s;

    if (send_signal)
        emit viewSpaceChanged(space_);

    update();
}

void GeneralSequenceFloatView::setSequence(const SequenceFloat * s)
{
    if (s == sequence_)
        return;

    sequence_ = s;
    static_cast<SequenceCurveData*>(curveData_)->sequence = s;
    over_->setSequence(s);
    curve_->setCurveData(s ? curveData_ : 0);

    update();
}


void GeneralSequenceFloatView::paintEvent(QPaintEvent * e)
{
    QPainter p(this);

    p.setPen(Qt::NoPen);
    p.setBrush(brushBack_);
    p.drawRect(e->rect());


    grid_->setViewSpace(space_);
    grid_->paint(p, e->rect());

    curve_->setViewSpace(space_);
    curve_->paint(p, e->rect());

    over_->setViewSpace(space_);
    over_->paint(p, e->rect());
}


} // namespace GUI
} // namespace MO
