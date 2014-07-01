/** @file sequencefloatview.cpp

    @brief float sequence editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLayout>
#include <QLabel>

#include "sequencefloatview.h"
#include "timeline1dview.h"
#include "ruler.h"
#include "math/timeline1d.h"
#include "object/sequencefloat.h"

namespace MO {
namespace GUI {


SequenceFloatView::SequenceFloatView(QWidget *parent) :
    SequenceView    (parent),
    sequence_       (0),
    timeline_       (0)
{
    //createTimeline_();
    //setSequenceWidget_(timeline_);
}

void SequenceFloatView::createTimeline_()
{
    if (timeline_)
        return;

    auto tl = new MATH::Timeline1D;
    for (int i=0; i<20; ++i)
        tl->add((Double)rand()/RAND_MAX * 10.0, (Double)rand()/RAND_MAX, MATH::Timeline1D::Point::SYMMETRIC);
    tl->setAutoDerivative();

    timeline_ = new Timeline1DView(tl, this);
    timeline_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    connect(timeline_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), SLOT(updateViewSpace_(UTIL::ViewSpace)));
}

void SequenceFloatView::setSequence(SequenceFloat * s)
{
    bool different = sequence_ != s;
    sequence_ = s;

    setSequence_(s);

    if (different)
    {
        if (!sequence_)
            clearSettingsWidgets_();
        else
            createSettingsWidgets_();
    }
}

void SequenceFloatView::setViewSpace(const UTIL::ViewSpace & v)
{
    timeline_->setViewSpace(v);
    updateViewSpace_(v);
}

void SequenceFloatView::createSettingsWidgets_()
{
    clearSettingsWidgets_();


    auto w = newSetting(tr("Mode"));
    auto mode = new QComboBox(this);
    w->layout()->addWidget(mode);
    mode->addItem(tr("Oscillator"), QVariant(0));
    mode->addItem(tr("Equation"), QVariant(0));
    mode->addItem(tr("Timeline"), QVariant(0));

    addSettingsWidget_(w);
}

} // namespace GUI
} // namespace MO
