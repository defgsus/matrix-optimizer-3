/** @file sequencefloatview.cpp

    @brief float sequence editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequencefloatview.h"
#include "timeline1dview.h"
#include "ruler.h"
#include "math/timeline1d.h"
#include "object/sequencefloat.h"
#include "object/scene.h"
#include "io/error.h"
#include "io/log.h"
#include "generalsequencefloatview.h"
#include "util/scenesettings.h"
#include "painter/valuecurve.h"
#include "painter/sequenceoverpaint.h"

namespace MO {
namespace GUI {

namespace {
    class SequenceCurveData : public PAINTER::ValueCurveData
    {
    public:
        const SequenceFloat * sequence;
        Double value(Double time) const
            { return sequence->value(sequence->realStart() + time, MO_GUI_THREAD); }
    };
}


SequenceFloatView::SequenceFloatView(QWidget *parent) :
    SequenceView    (parent),
    sequence_       (0),
    timeline_       (0),
    seqView_        (0),
    sequenceCurveData_(0)
{
    updateSequence_();
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
}

void SequenceFloatView::createSequenceView_()
{
    if (seqView_)
        return;

    seqView_ = new GeneralSequenceFloatView(this);
    seqView_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    connect(seqView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), SLOT(updateViewSpace_(UTIL::ViewSpace)));
}

void SequenceFloatView::setSequence(SequenceFloat * s)
{
    MO_DEBUG_GUI("SequenceFloatView::setSequence(" << s << ") sequence_ = " << sequence_);

    bool different = sequence_ != s;
    sequence_ = s;

    setSequence_(s);

    if (different)
    {
        updateSequence_();
    }
}

void SequenceFloatView::updateSequence_()
{
    // XXX This is called quite often!!

    if (sequence_ && sequence_->sequenceType() == SequenceFloat::ST_TIMELINE)
    {
        MO_ASSERT(sequence_->timeline(), "No timeline in SequenceFloat with timeline mode");

        createTimeline_();
        timeline_->setTimeline(sequence_->timeline());

        MO_ASSERT(timeline_->sequenceOverpaint(), "No sequence-overpainter in Timeline1DView");
        timeline_->sequenceOverpaint()->setSequence(sequence_);
        MO_ASSERT(timeline_->sequenceCurvePainter(), "No sequence-curve-painter in Timeline1DView");
        timeline_->sequenceCurvePainter()->setCurveData( sequenceCurveData_ );
        static_cast<SequenceCurveData*>(sequenceCurveData_)->sequence = sequence_;

        setSequenceWidget_(timeline_);
        if (seqView_ && seqView_->isVisible())
            seqView_->setVisible(false);
        timeline_->setVisible(true);

    }
    else
    {
        createSequenceView_();
        seqView_->setSequence(sequence_);
        setSequenceWidget_(seqView_);
        if (timeline_ && timeline_->isVisible())
            timeline_->setVisible(false);
        seqView_->setVisible(true);
    }

    setViewSpace(viewSpace());

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

    // save the current viewspace
    if (sequence_)
        sceneSettings()->setViewSpace(sequence_, v);

    updateViewSpace_(v);
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
}


} // namespace GUI
} // namespace MO
