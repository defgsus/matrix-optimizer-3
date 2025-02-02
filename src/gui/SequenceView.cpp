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

#include "SequenceView.h"
#include "SequenceFloatView.h"
#include "Ruler.h"
#include "object/Scene.h"
#include "object/control/Sequence.h"
#include "object/control/SequenceFloat.h"
#include "object/control/Clip.h"
#include "object/util/ObjectEditor.h"
#include "io/error.h"
#include "widget/DoubleSpinBox.h"
#include "widget/TimeBar.h"
#include "util/SceneSettings.h"
#include "io/log_gui.h"
#include "io/log_param.h"

namespace MO {
namespace GUI {

SequenceView::SequenceView(QWidget *parent) :
    QWidget         (parent),
    sequence_       (0),
    valueFloat_     (0),
    scene_          (0),
    sceneSettings_  (0),
    grid_           (new QGridLayout(this)),
    rulerX_         (new Ruler(this)),
    rulerY_         (new Ruler(this)),
    seqFloatView_   (0)
{
    setObjectName("_SequenceView");

    setFocusPolicy(Qt::StrongFocus);

    grid_->setMargin(1);
    grid_->setContentsMargins(1,1,1,1);
    grid_->setSpacing(2);
    // make sequence the biggest component
    grid_->setColumnStretch(1, 10);

    grid_->addWidget(rulerX_, 0, 1);
    grid_->addWidget(rulerY_, 1, 0);

    // set ruler properties

    rulerX_->setFixedHeight(34);
    rulerX_->setOptions(Ruler::O_EnableAllX);

    rulerY_->setFixedWidth(60);
    rulerY_->setOptions(Ruler::O_EnableAllY);

    // connect rulers to class
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SLOT(setViewSpace(UTIL::ViewSpace)));

    // connect rulers to each other
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX_, SLOT(setViewSpace(UTIL::ViewSpace)));
    //connect(rulerY_, SIGNAL(fitRequest()),

    // pass viewSpaceChanged to this classes signal
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SLOT(onViewSpaceChanged_(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SLOT(onViewSpaceChanged_(UTIL::ViewSpace)));
    //connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));

    // connect ruler click to scene time
    connect(rulerX_, SIGNAL(doubleClicked(Double)), this, SLOT(rulerXClicked_(Double)));
    // value ruler click to focus on value range
    connect(rulerY_, SIGNAL(doubleClicked(Double)), this, SLOT(adjustViewspaceVertically()));

    // playbar
    playBar_ = new TimeBar(this);
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), playBar_, SLOT(setViewspace(UTIL::ViewSpace)));
    connect(playBar_, SIGNAL(timeChanged(Double)), this, SIGNAL(sceneTimeChanged(Double)));


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

    // initialize some viewspace
    rulerX_->setViewSpace( UTIL::ViewSpace(0, -1, 60, 2) );

    setSceneTime(0);
}

void SequenceView::setScene(Scene *scene)
{
    if (scene && scene->editor()
        && (!scene_ || scene->editor() != scene_->editor()))
    {
        scene_ = scene;
        connect(scene_->editor(), SIGNAL(sequenceChanged(MO::Sequence*)),
                this, SLOT(onSequenceChanged_(MO::Sequence*)));
        connect(scene_->editor(), SIGNAL(parametersChanged(MO::Object*)),
                this, SLOT(onParametersChanged_(MO::Object*)));
    }
}

void SequenceView::setSceneTime(Double time)
{
    playBar_->setTime(time);

    // sequence behaviour in containing clip
    if (sequence_ && sequence_->parentClip())
    {
        playBar_->setTimeOffset(-sequence_->realStart());
        playBar_->setActive(sequence_->parentClip()->isPlaying());
    }
}

const UTIL::ViewSpace& SequenceView::viewSpace() const
{
    return rulerX_->viewSpace();
}

void SequenceView::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);

    // update viewspace
    rulerX_->setViewSpace( viewSpace(), true);

    playBar_->setContainingRect(
        QRect(rulerX_->pos(), QSize(rulerX_->width(), grid_->geometry().height()))
                );
}

void SequenceView::focusInEvent(QFocusEvent * e)
{
    QWidget::focusInEvent(e);

    emit clicked();
}

void SequenceView::setSequence(Sequence * s)
{
    setSequence_(s);
}

void SequenceView::setSequence_(Sequence * s)
{
    MO_DEBUG_GUI("SequenceView::setSequence(" << s << ") baseSequence_ = " << sequence_);

    MO_ASSERT(sceneSettings_, "SequenceView::setSequence() without SceneSettings");

    bool different = sequence_ != s;

    sequence_ = s;
    valueFloat_ = 0;

    if (different)
        updateSequenceWidget_();

    if (different && sequence_)
    {
        // adjust playbar offset to local time
        if (!sequence_->parentClip())
            playBar_->setTimeOffset(-sequence_->start());
        else
        {
            playBar_->setTimeOffset(-sequence_->realStart());
            playBar_->setActive(sequence_->parentClip()->isPlaying());
        }

        // get viewspace from previous use
        setViewSpace(sceneSettings_->getViewSpace(sequence_));
    }
}

void SequenceView::setValueFloat(ValueFloatInterface *iface)
{
    bool different = valueFloat_ != iface;

    sequence_ = 0;
    valueFloat_ = iface;

    if (different)
    {
        updateSequenceWidget_();
        playBar_->setTimeOffset(0.);

        setViewSpace(viewSpace());
    }
}

void SequenceView::setNothing()
{
    MO_DEBUG_GUI("SequenceView::setNothing()");

    bool change = sequence_ || valueFloat_;
    sequence_ = 0;
    valueFloat_ = 0;
    if (change)
    {
        updateSequenceWidget_();
        playBar_->setTimeOffset(0.);
    }
}

void SequenceView::updateSequenceWidget_()
{
    if (!sequence_ && !valueFloat_)
    {
        if (seqFloatView_)
        {
            seqFloatView_->setVisible(false);
            seqFloatView_->setSequence(0);
        }

        return;
    }

    if (sequence_ && sequence_->type() == Object::T_SEQUENCE_FLOAT)
    {
        createSeqFloatView_();
        seqFloatView_->setSequence(static_cast<SequenceFloat*>(sequence_));
        seqFloatView_->setVisible(true);
    }
    else
    if (valueFloat_)
    {
        createSeqFloatView_();
        seqFloatView_->setValueFloat(valueFloat_);
        seqFloatView_->setVisible(true);
    }

    playBar_->raise();
}

void SequenceView::createSeqFloatView_()
{
    if (!seqFloatView_)
    {
        seqFloatView_ = new SequenceFloatView(this);
        connect(seqFloatView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
                this, SLOT(onViewSpaceChanged_(UTIL::ViewSpace)));
        grid_->addWidget(seqFloatView_, 1, 1);
    }
}

void SequenceView::setViewSpace(const UTIL::ViewSpace & v)
{
    // own space
    updateViewSpace_(v);

    // sub-widgets
    if (seqFloatView_)
        seqFloatView_->setViewSpace(v);

}

void SequenceView::adjustViewspaceVertically()
{
    auto v = viewSpace();

    if (!sequence_)
    {
        v.setY(-1.);
        v.setScaleY(2.);
    }

    auto sf = dynamic_cast<ValueFloatInterface*>(sequence_);
    if (!sf)
        sf = dynamic_cast<ValueFloatInterface*>(valueFloat_);
    if (sf)
    {
        Double mi, ma,
               start = v.mapXTo(0.), end = v.mapXTo(1.);
        sf->getValueFloatRange(0, RenderTime(start, MO_GUI_THREAD), end - start, &mi, &ma);
        ma += 0.1; mi -= 0.1;
        Double delta = (ma - mi) / 50.;
        v.setY(mi - delta);
        v.setScaleY(ma - mi + delta * 2.);
    }

    setViewSpace(v);

    // save the current viewspace
    if (sequence_)
        sceneSettings()->setViewSpace(sequence_, v);
}

void SequenceView::onViewSpaceChanged_(const UTIL::ViewSpace & v)
{
    // save the current viewspace
    if (sequence_)
        sceneSettings()->setViewSpace(sequence_, v);

    updateViewSpace_(v);

    emit viewSpaceChanged(v);
}

void SequenceView::updateViewSpace_(const UTIL::ViewSpace & v)
{

    rulerX_->setViewSpace(v);
    rulerY_->setViewSpace(v);
    //if (!baseSequence_)
        playBar_->setViewspace(v);
    /*else
    {
        UTIL::ViewSpace vs(v);
        vs.setScaleX(vs.scaleX() / baseSequence_->speed());
        playBar_->setViewspace(vs);
    }*/
}



void SequenceView::rulerXClicked_(Double time)
{
    if (sequence_)
        time += sequence_->realStart();

    emit sceneTimeChanged(time);
}



void SequenceView::onParametersChanged_(Object* o)
{
    if (o == sequence_ || (void*)o == (void*)valueFloat_)
        onSequenceChanged_(sequence_);
}


void SequenceView::onSequenceChanged_(Sequence * s)
{
    MO_DEBUG_PARAM("SequenceView::sequenceTimeChanged(" << s
                   << ") baseSequence_=" << sequence_
                   //<< " defaultSettingsAvailable_=" << defaultSettingsAvailable_
                   );
    Q_UNUSED(s);

    // update playbar
    if (sequence_ && sequence_ == s)
    {
        playBar_->setTimeOffset(-sequence_->realStart());
        playBar_->setMinimumTime(-sequence_->start());
        if (sequence_->parentClip())
            playBar_->setActive(sequence_->parentClip()->isPlaying());
    }

    // repaint float view
    if (seqFloatView_ && seqFloatView_->isVisible())
        seqFloatView_->updateSequence();

    update();
}

} // namespace GUI
} // namespace MO
