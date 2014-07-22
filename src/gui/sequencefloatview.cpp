/** @file sequencefloatview.cpp

    @brief float sequence editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/


#include <QComboBox>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>

#include "sequencefloatview.h"
#include "timeline1dview.h"
#include "ruler.h"
#include "math/timeline1d.h"
#include "object/sequencefloat.h"
#include "object/scene.h"
#include "io/error.h"
#include "io/log.h"
#include "generalsequencefloatview.h"
#include "math/waveform.h"
#include "widget/doublespinbox.h"
#include "widget/equationeditor.h"


namespace MO {
namespace GUI {


SequenceFloatView::SequenceFloatView(QWidget *parent) :
    SequenceView    (parent),
    sequence_       (0),
    timeline_       (0),
    seqView_        (0),
    wFreq_          (0)
{
    updateSequence_();
}

void SequenceFloatView::createTimeline_()
{
    if (timeline_)
        return;

    timeline_ = new Timeline1DView(0, this);
    timeline_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    connect(timeline_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
                 this, SLOT(updateViewSpace_(UTIL::ViewSpace)));
    connect(timeline_, SIGNAL(statusTipChanged(QString)),
                 this, SIGNAL(statusTipChanged(QString)));

    timeline_->setLockFunctions(
                [this](){ sequence_->sceneObject()->beginTimelineChange(sequence_); },
                [this](){ sequence_->sceneObject()->endTimelineChange(); } );
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

        if (!sequence_)
        {
            clearSettingsWidgets_();
            // test for presence of all widgets
            wFreq_ = 0;
        }
        else
            createSettingsWidgets_();
    }
}

void SequenceFloatView::updateSequence_()
{
    if (sequence_ && sequence_->mode() == SequenceFloat::ST_TIMELINE)
    {
        MO_ASSERT(sequence_->timeline(), "No timeline in SequenceFloat with timeline mode");

        createTimeline_();
        timeline_->setTimeline(sequence_->timeline());
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

    updateWidgets_();
    update();
}

void SequenceFloatView::setViewSpace(const UTIL::ViewSpace & v)
{
    if (timeline_)
        timeline_->setViewSpace(v);
    if (seqView_)
        seqView_->setViewSpace(v);

    updateViewSpace_(v);
}

void SequenceFloatView::createSettingsWidgets_()
{
    clearSettingsWidgets_();

    Scene * scene = sequence_->sceneObject();
    MO_ASSERT(scene, "no scene for Sequence in SequenceFloatView");

    // sequence mode
    auto w = newSetting(tr("sequence"));
    w->setStatusTip(tr("Selects the type of the sequence function"));
    auto mode = new QComboBox(this);
    w->layout()->addWidget(mode);
    for (int i=0; i<SequenceFloat::ST_MAX; ++i)
    {
        mode->addItem(SequenceFloat::sequenceTypeName[i]);
    }
    mode->setCurrentIndex(sequence_->mode());
    connect(mode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    [this, scene](int index)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setMode((SequenceFloat::SequenceType)index);
    });

    // oscillator mode
    w = wOscMode_ = newSetting(tr("oscillator"));
    w->setStatusTip(tr("Selects the type of oscillator waveform"));
    mode = new QComboBox(this);
    w->layout()->addWidget(mode);
    for (int i=0; i<MATH::Waveform::T_MAX_TYPES; ++i)
    {
        mode->addItem(MATH::Waveform::typeNames[i]);
    }
    mode->setCurrentIndex(sequence_->oscillatorMode());
    connect(mode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    [this, scene](int index)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setOscillatorMode((MATH::Waveform::Type)index);
    });

    // offset
    w = newSetting(tr("value\noffset"));
    w->setStatusTip(tr("This value is always added to the output of the sequence"));
    auto spin = new DoubleSpinBox(this);
    w->layout()->addWidget(spin);
    spin->setDecimals(5);
    spin->setRange(-1e8, 1e8);
    spin->setValue(sequence_->offset());
    spin->setSingleStep(0.1);
    connect(spin, &DoubleSpinBox::valueChanged,
    [this, scene](Double val)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setOffset(val);
    });

    // amplitude
    w = wAmp_ = newSetting(tr("amplitude"));
    w->setStatusTip(tr("The output of the sequence is multiplied by this value"));
    spin = new DoubleSpinBox(this);
    w->layout()->addWidget(spin);
    spin->setDecimals(5);
    spin->setRange(-1e8, 1e8);
    spin->setValue(sequence_->amplitude());
    spin->setSingleStep(0.1);
    connect(spin, &DoubleSpinBox::valueChanged,
    [this, scene](Double val)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setAmplitude(val);
    });

    // frequency
    w = wFreq_ = newSetting(tr("frequency"));
    w->setStatusTip(tr("The frequency of the function in hertz (periods per second)"));
    spin = new DoubleSpinBox(this);
    w->layout()->addWidget(spin);
    spin->setDecimals(5);
    spin->setRange(-1e8, 1e8);
    spin->setSingleStep(0.1);
    spin->setValue(sequence_->frequency());
    connect(spin, &DoubleSpinBox::valueChanged,
    [this, scene](Double val)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setFrequency(val);
    });

    // phase
    w = wPhase_ = newSetting(tr("phase"));
    w->setStatusTip(tr("Phase (time shift) of the function, either in degree [0,360] or periods [0,1]"));
    spin = new DoubleSpinBox(this);
    w->layout()->addWidget(spin);
    spin->setDecimals(5);
    spin->setRange(-1e8, 1e8);
    spin->setSingleStep(5);
    spin->setValue(sequence_->phase());
    connect(spin, &DoubleSpinBox::valueChanged,
    [this, scene](Double val)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setPhase(val);
    });

    // pulseWidth
    w = wPW_ = newSetting(tr("pulsewidth"));
    w->setStatusTip(tr("Pulsewidth of the waveform, describes the width of the positive edge"));
    spin = new DoubleSpinBox(this);
    w->layout()->addWidget(spin);
    spin->setDecimals(5);
    spin->setRange(MATH::Waveform::minPulseWidth(), MATH::Waveform::maxPulseWidth());
    spin->setSingleStep(0.025);
    spin->setValue(sequence_->pulseWidth());
    connect(spin, &DoubleSpinBox::valueChanged,
    [this, scene](Double val)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setPulseWidth(val);
    });

    // equation
    w = wEqu_ = newSetting(tr("equation"));
    w->setStatusTip(tr("The equation text can be a mix of functions, variables, numbers and operators"));
    auto text = wEquEdit_ = new EquationEditor(this);
    w->layout()->addWidget(text);
    text->setPlainText(sequence_->equationText());
    text->setParser(sequence_->equation());
    connect(text, &EquationEditor::equationChanged,
    [this, scene, text]()
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setEquationText(text->toPlainText());
    });

    // always use frequency
    w = wPhaseDeg_ = newSetting(tr("phase in degree"));
    w->setStatusTip(tr("Selects whether phase is given in "
                     "degree [0,360] or in unit range [0,1]"));
    auto cb = new QCheckBox(this);
    w->layout()->addWidget(cb);
    cb->setChecked(sequence_->phaseInDegree());
    connect(cb, &QCheckBox::stateChanged,
    [this, scene, cb]()
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setPhaseInDegree(cb->isChecked());
    });

    // always use frequency
    w = wUseFreq_ = newSetting(tr("use frequency"));
    w->setStatusTip(tr("Selects whether the function time "
                     "should be modified by frequency and phase, "
                     "as in oscillator mode."));
    cb = new QCheckBox(this);
    w->layout()->addWidget(cb);
    cb->setChecked(sequence_->useFrequency());
    connect(cb, &QCheckBox::stateChanged,
    [this, scene, cb]()
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setUseFrequency(cb->isChecked());
    });

    // enable loop overlap
    w = wLoopOverlapping_ = newSetting(tr("loop overlapping"));
    w->setStatusTip(tr("Selects the type of loop overlapping"));
    mode = new QComboBox(this);
    w->layout()->addWidget(mode);
    for (int i=0; i<SequenceFloat::LOT_MAX; ++i)
    {
        mode->addItem(SequenceFloat::loopOverlapModeName[i]);
    }
    mode->setCurrentIndex(sequence_->loopOverlapMode());
    connect(mode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    [this, scene](int index)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setLoopOverlapMode((SequenceFloat::LoopOverlapMode)index);
    });

    // overlap length
    w = wLoopOverlap_ = newSetting(tr("loop overlap"));
    w->setStatusTip(tr("Overlap of the loop window for smooth transitions (seconds)"));
    spin = new DoubleSpinBox(this);
    w->layout()->addWidget(spin);
    spin->setDecimals(4);
    spin->setMinimum(Sequence::minimumLength());
    spin->setSingleStep(0.1);
    spin->setValue(sequence_->loopOverlap());
    connect(spin, &DoubleSpinBox::valueChanged,
    [this, scene](Double val)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setLoopOverlap(val);
    });

    // overlap value offset
    w = wLoopOverlapOffset_ = newSetting(tr("overlap offset"));
    w->setStatusTip(tr("A value that is added to the blended value in the transition window"));
    spin = new DoubleSpinBox(this);
    w->layout()->addWidget(spin);
    spin->setRange(-1e8, 1e8);
    spin->setDecimals(4);
    spin->setSingleStep(1);
    spin->setValue(sequence_->loopOverlapOffset());
    connect(spin, &DoubleSpinBox::valueChanged,
    [this, scene](Double val)
    {
        ScopedSequenceChange lock(scene, sequence_);
        sequence_->setLoopOverlapOffset(val);
    });

    addSettingsWidget_(w);

    updateWidgets_();
}

void SequenceFloatView::updateWidgets_()
{
    if (!wFreq_)
        return;

    bool isConst = sequence_ && sequence_->mode() == SequenceFloat::ST_CONSTANT,
         isOsc = sequence_ && sequence_->mode() == SequenceFloat::ST_OSCILLATOR,
         isPW = isOsc && MATH::Waveform::supportsPulseWidth( sequence_->oscillatorMode() ),
         isEqu = sequence_ && sequence_->mode() == SequenceFloat::ST_EQUATION,
         isTL = sequence_ && sequence_->mode() == SequenceFloat::ST_TIMELINE,
         useFreq = sequence_ && sequence_->useFrequency(),
         isLoop = sequence_ && sequence_->looping(),
         isLoopOverlap = isLoop && sequence_->loopOverlapMode() != SequenceFloat::LOT_OFF;

    wOscMode_->setVisible(isOsc);
    wAmp_->setVisible(!isConst);
    wFreq_->setVisible(isOsc || isEqu || useFreq);
    wPhase_->setVisible(isOsc || isEqu || useFreq);
    wPhaseDeg_->setVisible(isOsc || isEqu || useFreq);
    wPW_->setVisible(isPW || isEqu);
    wEqu_->setVisible(isEqu);
    wUseFreq_->setVisible(isEqu || isTL);
    wLoopOverlapping_->setVisible(isLoop);
    wLoopOverlap_->setVisible(isLoopOverlap);
    wLoopOverlapOffset_->setVisible(isLoopOverlap);
    if (isEqu && wEquEdit_->assignedParser() != sequence_->equation())
        wEquEdit_->setParser(sequence_->equation());

}

} // namespace GUI
} // namespace MO
