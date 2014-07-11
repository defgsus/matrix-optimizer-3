/** @file sequenceview.cpp

    @brief Sequence editor base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/


#include <QDebug>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QCheckBox>
#include <QScrollBar>

#include "sequenceview.h"
#include "ruler.h"
#include "object/scene.h"
#include "object/sequence.h"
#include "io/error.h"
#include "widget/doublespinbox.h"
#include "widget/timebar.h"
#include "io/log.h"

namespace MO {
namespace GUI {

SequenceView::SequenceView(QWidget *parent) :
    QWidget         (parent),
    baseSequence_   (0),
    grid_           (new QGridLayout(this)),
    rulerX_         (new Ruler(this)),
    rulerY_         (new Ruler(this)),
    settings_       (0),
    defaultSettingsAvailable_(false)
{
    grid_->setMargin(1);
    grid_->setContentsMargins(1,1,1,1);
    grid_->setSpacing(2);
    // make sequence the biggest component
    grid_->setColumnStretch(2, 10);

    //grid_->addWidget(timelineView_, 1, 1);
    grid_->addWidget(rulerX_, 0, 2);
    grid_->addWidget(rulerY_, 1, 1);

    // set ruler properties

    rulerX_->setFixedHeight(34);
    rulerX_->setOptions(Ruler::O_EnableAllX);

    rulerY_->setFixedWidth(60);
    rulerY_->setOptions(Ruler::O_EnableAllY);

    // connect rulers to class
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), SLOT(setViewSpace(UTIL::ViewSpace)));

    // connect rulers to each other
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX_, SLOT(setViewSpace(UTIL::ViewSpace)));

    // pass viewSpaceChanged to this classes signal
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));
    //connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));

    // connect ruler click to scene time
    connect(rulerX_, SIGNAL(doubleClicked(Double)), this, SLOT(rulerXClicked_(Double)));

    // playbar
    playBar_ = new TimeBar(this);
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), playBar_, SLOT(setViewspace(UTIL::ViewSpace)));
    connect(playBar_, SIGNAL(timeChanged(Double)), this, SIGNAL(sceneTimeChanged(Double)));

    // ---- container for settings ----

    settings_ = new QScrollArea(this);
    // XXX this value should be estimated!!
    settings_->setMinimumWidth(250);
    grid_->addWidget(settings_, 0, 0, 2, 1);

    auto w = new QWidget(settings_);
    w->setObjectName("_settings");
    w->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    settings_->setWidget(w);

    settingsLayout_ = new QVBoxLayout(w);
    settingsLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

    customSettingsContainer_ = new QWidget(settings_);
    customSettingsContainer_->setObjectName("_custom_settings");
    settingsLayout_->addWidget(customSettingsContainer_);
    (new QVBoxLayout(customSettingsContainer_))->setMargin(0);

    defaultSettingsContainer_ = new QWidget(settings_);
    defaultSettingsContainer_->setObjectName("_default_settings");
    settingsLayout_->addWidget(defaultSettingsContainer_);
    (new QVBoxLayout(defaultSettingsContainer_))->setMargin(0);

    // connect fit-request from ruler to timeline's fit-to-view
    /*connect(rulerX_, &Ruler::fitRequest, [=]()
    {
        if (timelineView_->options() & Timeline1DView::O_ZoomViewX)
        {
            if (timelineView_->isSelected())
                timelineView_->fitSelectionToView(true, false);
            else
                timelineView_->fitToView(true, false);
        }
    });
    connect(rulerY_, &Ruler::fitRequest, [=]()
    {
        if (timelineView_->options() & Timeline1DView::O_ZoomViewY)
        {
            if (timelineView_->isSelected())
                timelineView_->fitSelectionToView(false, true);
            else
                timelineView_->fitToView(false, true);
        }
    });

    // update rulers
    timelineView_->setViewSpace(timelineView_->viewSpace(), true);
    */

    rulerX_->setViewSpace( UTIL::ViewSpace(0, -1, 60, 2) );

    setSceneTime(0);
}

void SequenceView::setSceneTime(Double time)
{
    playBar_->setTime(time);
}

void SequenceView::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);

    rulerX_->setViewSpace( rulerX_->viewSpace(), true);

    playBar_->setContainingRect(
        QRect(rulerX_->pos(), QSize(rulerX_->width(), grid_->geometry().height()))
                );
}

void SequenceView::setSequence_(Sequence * s)
{
    bool different = baseSequence_ != s;

    // remove old connection !!
    if (different && baseSequence_)
    {
        disconnect(baseSequence_, SIGNAL(timeChanged(MO::Sequence*)),
                   this, SLOT(sequenceTimeChanged_(MO::Sequence*)));
    }

    baseSequence_ = s;

    if (different)
    {
        if (!baseSequence_)
            clearDefaultSettingsWidgets_();
        else
        {
            createDefaultSettingsWidgets_();
            connect(baseSequence_, SIGNAL(timeChanged(MO::Sequence*)),
                    this, SLOT(sequenceTimeChanged_(MO::Sequence*)));

            playBar_->setTimeOffset(-baseSequence_->start());
        }
    }
}

void SequenceView::updateViewSpace_(const UTIL::ViewSpace & v)
{
    rulerX_->setViewSpace(v);
    rulerY_->setViewSpace(v);
    playBar_->setViewspace(v);
}


void SequenceView::setSequenceWidget_(QWidget * w)
{
    grid_->addWidget(w, 1, 2);

    playBar_->raise();
}

void SequenceView::rulerXClicked_(Double time)
{
    if (baseSequence_)
        time += baseSequence_->start();

    emit sceneTimeChanged(time);
}


// --------------------- settings widgets ---------------------------

QWidget * SequenceView::newContainer_(const QString& name)
{
    auto w = new QWidget(this);
    w->setObjectName("_" + name);

    auto l = new QHBoxLayout(w);
    l->setMargin(0);

    l->addWidget(new QLabel(name, w));

    return w;
}

QWidget * SequenceView::newSetting(const QString & name)
{
    auto w = newContainer_(name);
    addSettingsWidget_(w);

    return w;
}

QWidget * SequenceView::newDefaultSetting_(const QString & name)
{
    auto w = newContainer_(name);
    defaultSettingsContainer_->layout()->addWidget(w);
    defaultSettingsWidgets_.append(w);

    return w;
}


void SequenceView::clearSettingsWidgets_()
{
    for (auto w : customSettingsWidgets_)
    {
        // to shrink the scrollarea viewport
        w->setVisible(false);
        w->deleteLater();
    }

    squeezeView_();

    customSettingsWidgets_.clear();
}

void SequenceView::addSettingsWidget_(QWidget * w)
{
    customSettingsContainer_->layout()->addWidget(w);
    customSettingsWidgets_.append(w);
}

void SequenceView::clearDefaultSettingsWidgets_()
{
    defaultSettingsAvailable_ = false;

    for (auto w : defaultSettingsWidgets_)
    {
        // to shrink the scrollarea viewport
        w->setVisible(false);
        w->deleteLater();
    }

    squeezeView_();

    defaultSettingsWidgets_.clear();
}

void SequenceView::createDefaultSettingsWidgets_()
{
    clearDefaultSettingsWidgets_();

    if (!baseSequence_)
        return;

    QWidget * w;

    Scene * scene = baseSequence_->sceneObject();
    MO_ASSERT(scene, "no scene for Sequence in SequenceView");

    // hline
    auto f = new QFrame(this);
    f->setFrameShape(QFrame::HLine);
    defaultSettingsContainer_->layout()->addWidget(f);
    defaultSettingsWidgets_.append(f);

#define MO__SCENE_PARAM(spin__, getter__, setter__, min__, desc__, status__) \
    w = newDefaultSetting_(desc__);                         \
    w->setStatusTip(status__);                              \
    w->layout()->addWidget(spin__ = new DoubleSpinBox(w));  \
    spin__->setDecimals(4);                                 \
    spin__->setMinimum(min__);                              \
    spin__->setMaximum(MO_MAX_TIME);                        \
    spin__->setValue(baseSequence_->getter__());            \
    connect(spin__,                                         \
     &DoubleSpinBox::valueChanged, [this, scene](Double v)  \
    {                                                       \
        ScopedSequenceChange lock(scene, baseSequence_);    \
        baseSequence_->setter__(v);                         \
    });

#define MO__SCENE_PARAM_CB(cb__, getter__, setter__, desc__, status__)\
    w = newDefaultSetting_(desc__);                         \
    w->setStatusTip(status__);                              \
    w->layout()->addWidget(cb__ = new QCheckBox(w));        \
    cb__->setChecked(baseSequence_->getter__());            \
    connect(cb__,                                           \
     &QCheckBox::stateChanged, [this, scene](int v)         \
    {                                                       \
        ScopedSequenceChange lock(scene, baseSequence_);    \
        baseSequence_->setter__(v == Qt::Checked);          \
    });

    MO__SCENE_PARAM(spinStart_, start, setStart, 0, tr("start time"),
                    tr("Global start of the sequence in seconds."));
    MO__SCENE_PARAM(spinLength_, length, setLength, Sequence::minimumLength(), tr("length"),
                    tr("Length of the sequence in seconds"));
    MO__SCENE_PARAM(spinEnd_, end, setEnd, 0, tr("end time"),
                    tr("Gobal end time of the sequences in seconds"));
    MO__SCENE_PARAM(spinTimeOffset_, timeOffset, setTimeOffset, -MO_MAX_TIME, tr("time offset"),
                    tr("Time offset into the sequence data in seconds"));
    MO__SCENE_PARAM(spinSpeed_, speed, setSpeed, Sequence::minimumSpeed(), tr("speed"),
                    tr("Speed multiplier for the whole sequence, e.g. 2.0 is double speed, 0.5 is half speed"));
    MO__SCENE_PARAM_CB(cbLooping_, looping, setLooping, tr("looping"),
                    tr("Selects wether the contents of the sequence should be looped between "
                       "'loop start' and 'loop end'"));
    MO__SCENE_PARAM(spinLoopStart_, loopStart, setLoopStart, 0, tr("loop start"),
                    tr("Local start time of loop in seconds"));
    wLoopStart_ = w;
    MO__SCENE_PARAM(spinLoopLength_, loopLength, setLoopLength,
                    Sequence::minimumLength(), tr("loop length"),
                    tr("Local length of loop in seconds"));
    wLoopLength_ = w;
    MO__SCENE_PARAM(spinLoopEnd_, loopEnd, setLoopEnd, 0, tr("loop end"),
                    tr("Local end time of loop in seconds."));
    wLoopEnd_ = w;

    bool loop = baseSequence_->looping();
    wLoopStart_->setVisible(loop);
    wLoopEnd_->setVisible(loop);
    wLoopLength_->setVisible(loop);

#undef MO__SCENE_PARAM
#undef MO__SCENE_PARAM_CB
#undef MO__SCENE_PARAM_ONCHANGE


    defaultSettingsAvailable_ = true;
}

void SequenceView::sequenceTimeChanged_(Sequence * s)
{
    MO_DEBUG_PARAM("SequenceView::sequenceTimeChanged(" << s
                   << ") baseSequence_=" << baseSequence_
                   << " defaultSettingsAvailable_=" << defaultSettingsAvailable_);

    if (!defaultSettingsAvailable_ || !baseSequence_
        || s != baseSequence_)
        return;

    playBar_->setTimeOffset(-baseSequence_->start());
    playBar_->setMinimumTime(-baseSequence_->start());

    spinStart_->setValue( baseSequence_->start() );
    spinLength_->setValue( baseSequence_->length() );
    spinEnd_->setValue( baseSequence_->end() );
    spinLoopStart_->setValue( baseSequence_->loopStart() );
    spinLoopLength_->setValue( baseSequence_->loopLength() );
    spinLoopEnd_->setValue( baseSequence_->loopEnd() );
    spinTimeOffset_->setValue( baseSequence_->timeOffset() );
    spinSpeed_->setValue( baseSequence_->speed() );
    // XXX this would cause infinite recursion but
    // QCheckBox only emits when value actually changes
    // XXX Probably we don't need it here anyway since
    // looping does not change from outside the editor yet
    cbLooping_->setChecked( baseSequence_->looping() );

    wLoopStart_->setVisible( baseSequence_->looping() );
    wLoopEnd_->setVisible( baseSequence_->looping() );
    wLoopLength_->setVisible( baseSequence_->looping() );

    squeezeView_();
    update();
}

void SequenceView::squeezeView_()
{
    const int h = settings_->verticalScrollBar()->sliderPosition();

    // squeeze container
    defaultSettingsContainer_->layout()->activate();
    customSettingsContainer_->layout()->activate();
    settings_->widget()->layout()->activate();
    settings_->widget()->setGeometry(QRect(0,0,1,1));

    settings_->ensureWidgetVisible(settings_->widget()->focusWidget());

    // little hack to update the viewport to the slider position
    // (it won't do it without)
    settings_->verticalScrollBar()->setSliderPosition(h-1);
    settings_->verticalScrollBar()->setSliderPosition(h);
}

} // namespace GUI
} // namespace MO
