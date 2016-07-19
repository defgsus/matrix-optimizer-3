/** @file generalsequencefloatview.cpp

    @brief Display for float sequences, except timelines

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QPaintEvent>
#include <QPainter>


#include "GeneralSequenceFloatView.h"
#include "painter/Grid.h"
#include "painter/ValueCurve.h"
#include "painter/SequenceOverpaint.h"
#include "object/control/SequenceFloat.h"
#include "object/control/Clip.h"
#include "object/interface/ValueFloatInterface.h"

namespace MO {
namespace GUI {

namespace {

    /** wrapper for SequenceFloat */
    class SequenceCurveData : public PAINTER::ValueCurveData
    {
    public:
        const SequenceFloat * sequence;
        Double value(Double time) const
        {
            return sequence->valueFloat(0, RenderTime(sequence->realStart() + time, MO_GUI_THREAD));
        }
    };

    /** wrapper for ValueFloatInterface */
    class ValueCurveData : public PAINTER::ValueCurveData
    {
    public:
        const ValueFloatInterface * iface;
        Double value(Double time) const
            { return iface->valueFloat(0, RenderTime(time, MO_GUI_THREAD)); }
    };

}


GeneralSequenceFloatView::GeneralSequenceFloatView(QWidget *parent) :
    QWidget         (parent),
    sequence_       (0),
    grid_           (new PAINTER::Grid(this)),
    curve_          (new PAINTER::ValueCurve(this)),
    curveDataSeq_   (new SequenceCurveData),
    curveDataValue_ (new ValueCurveData),
    over_           (new PAINTER::SequenceOverpaint(this)),

    brushBack_      (QColor(50,50,50))
{
}

GeneralSequenceFloatView::~GeneralSequenceFloatView()
{
    delete curveDataValue_;
    delete curveDataSeq_;
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
    valueFloat_ = 0;
    static_cast<SequenceCurveData*>(curveDataSeq_)->sequence = s;
    over_->setSequence(s);
    curve_->setCurveData(s ? curveDataSeq_ : 0);

    update();
}

void GeneralSequenceFloatView::setValueFloat(const ValueFloatInterface * iface)
{
    if (iface == valueFloat_)
        return;

    sequence_ = 0;
    valueFloat_ = iface;
    static_cast<ValueCurveData*>(curveDataValue_)->iface = iface;
    over_->setSequence(0);
    curve_->setCurveData(iface ? curveDataValue_ : 0);

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
