/** @file sequencefloatview.cpp

    @brief float sequence editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QLayout>

#include "SequenceFloatView.h"
#include "SequenceView.h"
#include "Timeline1dView.h"
#include "Ruler.h"
#include "GeneralSequenceFloatView.h"
#include "painter/ValueCurve.h"
#include "painter/SequenceOverpaint.h"
#include "object/control/SequenceFloat.h"
#include "object/Scene.h"
#include "math/Timeline1d.h"
#include "io/error.h"
#include "io/log_gui.h"

namespace MO {
namespace GUI {

namespace {

    /** wrapper for SequenceFloat */
    class SequenceCurveData : public PAINTER::ValueCurveData
    {
    public:
        const SequenceFloat * sequence;
        Double value(Double time) const
            { return sequence->valueFloat(0, RenderTime(sequence->realStart() + time, MO_GUI_THREAD)); }
    };

}


SequenceFloatView::SequenceFloatView(SequenceView *parent) :
    QWidget         (parent),
    view_           (parent),
    sequence_       (0),
    valueFloat_     (0),
    timeline_       (0),
    seqView_        (0),
    sequenceCurveData_(0),
    layout_         (0)
{
    updateSequence();
}

SequenceFloatView::~SequenceFloatView()
{
    if (sequenceCurveData_)
        delete sequenceCurveData_;
}

void SequenceFloatView::createTimeline_()
{
    if (timeline_)
        return;

    timeline_ = new Timeline1DView(0, this);
    timeline_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);


    connect(timeline_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
                 this, SLOT(updateViewSpaceTl_(UTIL::ViewSpace)));
    connect(timeline_, SIGNAL(statusTipChanged(QString)),
                 this, SIGNAL(statusTipChanged(QString)));

    // locking for timeline editing
    timeline_->setLockFunctions(
                [this](){ sequence_->sceneObject()->beginTimelineChange(sequence_); },
                [this](){ sequence_->sceneObject()->endTimelineChange(); } );

    // install additional painters

    timeline_->setSequenceOverpaint(new PAINTER::SequenceOverpaint(timeline_) );

    if (!sequenceCurveData_)
        sequenceCurveData_ = new SequenceCurveData();
    timeline_->setSequenceCurvePainter(new PAINTER::ValueCurve(timeline_));
    timeline_->sequenceCurvePainter()->setCurveData(sequenceCurveData_);
    timeline_->sequenceCurvePainter()->setAlpha(80);
    timeline_->sequenceCurvePainter()->setOverpaint(1);

    layout_->addWidget(timeline_);
}

void SequenceFloatView::createSequenceView_()
{
    if (seqView_)
        return;

    seqView_ = new GeneralSequenceFloatView(this);
    seqView_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    connect(seqView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));

    layout_->addWidget(seqView_);
}

void SequenceFloatView::setSequence(SequenceFloat * s)
{
    MO_DEBUG_GUI("SequenceFloatView::setSequence(" << s << ") sequence_ = " << sequence_);

    bool different = sequence_ != s;

    sequence_ = s;
    valueFloat_ = 0;

    if (different)
    {
        updateSequence();
    }
}

void SequenceFloatView::setValueFloat(ValueFloatInterface * iface)
{
    MO_DEBUG_GUI("SequenceFloatView::setValueFloat(" << iface << ") valueFloat_ = " << valueFloat_);

    bool different = sequence_ || valueFloat_ != iface;

    sequence_ = 0;
    valueFloat_ = iface;

    if (different)
    {
        updateSequence();
    }
}

void SequenceFloatView::updateSequence()
{
    // XXX This is called quite often!!

    if (!layout_)
    {
        layout_ = new QVBoxLayout(this);
        layout_->setContentsMargins(0,0,0,0);
    }

    bool handled = false;

    if (sequence_ && sequence_->sequenceType() == SequenceFloat::ST_TIMELINE)
    {
        MO_ASSERT(sequence_->timeline(), "No timeline in SequenceFloat with timeline mode");

        if (seqView_ && seqView_->isVisible())
            seqView_->setVisible(false);

        createTimeline_();
        timeline_->setTimeline(sequence_->timeline());

        MO_ASSERT(timeline_->sequenceOverpaint(), "No sequence-overpainter in Timeline1DView");
        timeline_->sequenceOverpaint()->setSequence(sequence_);
        MO_ASSERT(timeline_->sequenceCurvePainter(), "No sequence-curve-painter in Timeline1DView");
        timeline_->sequenceCurvePainter()->setCurveData( sequenceCurveData_ );
        static_cast<SequenceCurveData*>(sequenceCurveData_)->sequence = sequence_;

        timeline_->setVisible(true);
        handled = true;
    }
    else
    {
        if (timeline_)
        {
            timeline_->setTimeline(0);
            timeline_->setVisible(false);
        }
    }

    if (!handled && sequence_)
    {
        createSequenceView_();

        seqView_->setSequence(sequence_);
        seqView_->setVisible(true);
        handled = true;
    }

    if (!handled && valueFloat_)
    {
        createSequenceView_();

        seqView_->setValueFloat(valueFloat_);
        seqView_->setVisible(true);
        handled = true;
    }

    // XXX forgot why
    //setViewSpace(view_->viewSpace());

    update();
}

void SequenceFloatView::setViewSpace(const UTIL::ViewSpace & v)
{
    if (seqView_)// && seqView_->isVisible())
        seqView_->setViewSpace(v);

    if (timeline_)// && timeline_->isVisible())
    {
        if (sequence_)
        {
            // timeline's viewspace is special
            UTIL::ViewSpace s(v);
            s.mapToSequence(sequence_);
            timeline_->setViewSpace(s, v);
        }
        else
            timeline_->setViewSpace(v);
    }
}

void SequenceFloatView::updateViewSpaceTl_(const UTIL::ViewSpace & s)
{
    MO_ASSERT(sequence_, "");

    // unmap from special timeline's viewspace
    UTIL::ViewSpace v(s);
    v.mapFromSequence(sequence_);

    // and feed back in to timeline's overpainters
    // + update everyone else
    setViewSpace(v);

    // emit change
    emit viewSpaceChanged(v);
}


} // namespace GUI
} // namespace MO
