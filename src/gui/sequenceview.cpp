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
#include "object/clip.h"
#include "io/error.h"
#include "widget/doublespinbox.h"
#include "widget/timebar.h"
#include "util/scenesettings.h"
#include "io/log.h"

namespace MO {
namespace GUI {

SequenceView::SequenceView(QWidget *parent) :
    QWidget         (parent),
    baseSequence_   (0),
    scene_          (0),
    sceneSettings_  (0),
    grid_           (new QGridLayout(this)),
    rulerX_         (new Ruler(this)),
    rulerY_         (new Ruler(this))
{
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
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));
    //connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));

    // connect ruler click to scene time
    connect(rulerX_, SIGNAL(doubleClicked(Double)), this, SLOT(rulerXClicked_(Double)));

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
    if (scene_ != scene)
    {
        scene_ = scene;
        connect(scene_, SIGNAL(sequenceChanged(MO::Sequence*)),
                this, SLOT(onSequenceChanged_(MO::Sequence*)));
        connect(scene_, SIGNAL(parameterChanged(MO::Parameter*)),
                this, SLOT(onParameterChanged_(MO::Parameter*)));
    }
}

void SequenceView::setSceneTime(Double time)
{
    playBar_->setTime(time);
    // check for containing clip
    if (baseSequence_ && baseSequence_->parentClip())
    {
        playBar_->setTimeOffset(-baseSequence_->realStart());
        playBar_->setActive(baseSequence_->parentClip()->isPlaying());
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

void SequenceView::setSequence_(Sequence * s)
{
    MO_DEBUG_GUI("SequenceView::setSequence(" << s << ") baseSequence_ = " << baseSequence_);

    MO_ASSERT(sceneSettings_, "SequenceView::setSequence() without SceneSettings");

    bool different = baseSequence_ != s;

    baseSequence_ = s;

    if (different && baseSequence_)
    {
        // adjust playbar offset to local time
        if (!baseSequence_->parentClip())
            playBar_->setTimeOffset(-baseSequence_->start());
        else
        {
            playBar_->setTimeOffset(-baseSequence_->realStart());
            playBar_->setActive(baseSequence_->parentClip()->isPlaying());
        }
        // get viewspace from previous use
        setViewSpace(sceneSettings_->getViewSpace(baseSequence_));
    }
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


void SequenceView::setSequenceWidget_(QWidget * w)
{
    grid_->addWidget(w, 1, 1);

    playBar_->raise();
}

void SequenceView::rulerXClicked_(Double time)
{
    if (baseSequence_)
        time += baseSequence_->realStart();

    emit sceneTimeChanged(time);
}



void SequenceView::onParameterChanged_(Parameter * p)
{
    if (p->object() == baseSequence_)
        onSequenceChanged_(baseSequence_);
}

void SequenceView::onSequenceChanged_(Sequence * s)
{
    MO_DEBUG_PARAM("SequenceView::sequenceTimeChanged(" << s
                   << ") baseSequence_=" << baseSequence_
                   << " defaultSettingsAvailable_=" << defaultSettingsAvailable_);
    Q_UNUSED(s);

    playBar_->setTimeOffset(-baseSequence_->realStart());
    playBar_->setMinimumTime(-baseSequence_->start());
    if (baseSequence_->parentClip())
        playBar_->setActive(baseSequence_->parentClip()->isPlaying());

    updateSequence_();

    update();
}

} // namespace GUI
} // namespace MO
