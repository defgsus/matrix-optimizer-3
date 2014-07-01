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
#include "object/scene.h"
#include "io/error.h"
#include "ruler.h"

namespace MO {
namespace GUI {


SequenceFloatView::SequenceFloatView(QWidget *parent) :
    SequenceView    (parent),
    sequence_       (0),
    timeline_       (0),
    emptyRuler_     (0)
{
    updateSequence_();
}

void SequenceFloatView::createTimeline_()
{
    if (timeline_)
        return;

    timeline_ = new Timeline1DView(0, this);
    timeline_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    connect(timeline_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), SLOT(updateViewSpace_(UTIL::ViewSpace)));
}

void SequenceFloatView::createEmptyRuler_()
{
    if (emptyRuler_)
        return;

    emptyRuler_ = new Ruler(this);
    emptyRuler_->setOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    connect(emptyRuler_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), SLOT(updateViewSpace_(UTIL::ViewSpace)));
}

void SequenceFloatView::setSequence(SequenceFloat * s)
{
    bool different = sequence_ != s;
    sequence_ = s;

    setSequence_(s);

    if (different)
    {
        updateSequence_();

        if (!sequence_)
            clearSettingsWidgets_();
        else
            createSettingsWidgets_();
    }
}

void SequenceFloatView::updateSequence_()
{
    if (!sequence_)
    {
        createEmptyRuler_();
        setSequenceWidget_(emptyRuler_);
        return;
    }

    if (sequence_->mode() == SequenceFloat::ST_TIMELINE)
    {
        MO_ASSERT(sequence_->timeline(), "No timeline in SequenceFloat with timeline mode");

        createTimeline_();
        timeline_->setTimeline(sequence_->timeline());
        setSequenceWidget_(timeline_);
    }
}

void SequenceFloatView::setViewSpace(const UTIL::ViewSpace & v)
{
    if (timeline_)
        timeline_->setViewSpace(v);
    if (emptyRuler_)
        emptyRuler_->setViewSpace(v);

    updateViewSpace_(v);
}

void SequenceFloatView::createSettingsWidgets_()
{
    clearSettingsWidgets_();

    Scene * scene = sequence_->sceneObject();
    MO_ASSERT(scene, "no scene for Sequence in SequenceFloatView");


    auto w = newSetting(tr("Mode"));
    auto mode = new QComboBox(this);
    w->layout()->addWidget(mode);
    for (int i=0; i<SequenceFloat::ST_MAX; ++i)
    {
        mode->addItem(SequenceFloat::sequenceTypeName[i]);
    }
    mode->setCurrentIndex(sequence_->mode());
    connect(mode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    [=](int index)
    {
        scene->beginObjectChange(sequence_);
        sequence_->setMode((SequenceFloat::SequenceType)index);
        scene->endObjectChange();
    });

    addSettingsWidget_(w);
}



} // namespace GUI
} // namespace MO
