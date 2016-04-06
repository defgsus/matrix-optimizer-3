/** @file sequencer.cpp

    @brief Sequencer using TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/


#include <QGridLayout>
#include <QScrollBar>
#include <QWheelEvent>
#include <QToolButton>

#include "sequencer.h"
#include "trackheader.h"
#include "trackview.h"
#include "ruler.h"
#include "object/scene.h"
#include "object/control/track.h"
#include "object/control/sequence.h"
#include "io/log_gui.h"
#include "widget/timebar.h"
#include "widget/spacer.h"


namespace MO {
namespace GUI {


Sequencer::Sequencer(QWidget *parent) :
    QWidget(parent)
{
    setObjectName("_SequencerView");

    createWidgets_();
}



void Sequencer::createWidgets_()
{
    gridLayout_ = new QGridLayout(this);
    gridLayout_->setMargin(0);
    gridLayout_->setHorizontalSpacing(1);
    gridLayout_->setVerticalSpacing(1);

        // ruler seconds
        rulerSec_ = new Ruler(this);
        gridLayout_->addWidget(rulerSec_, 0, 2);
        rulerSec_->setOptions(Ruler::O_EnableAllX);
        rulerSec_->setFixedHeight(36);

        // ruler fps
        rulerFps_ = new Ruler(this);
        gridLayout_->addWidget(rulerFps_, 2, 2);
        rulerFps_->setOptions(Ruler::O_EnableAllX);
        rulerFps_->setFixedHeight(36);

        // track view
        trackView_ = new TrackView(this);
        gridLayout_->addWidget(trackView_, 1, 2);
        connect(trackView_, SIGNAL(sequenceSelected(Sequence*)),
                this, SLOT(onSequenceSelected_(Sequence*)));
        connect(trackView_, SIGNAL(sceneTimeChange(Double)),
                this, SIGNAL(sceneTimeChanged(Double)));

        // trackview filter
        auto filterBut = new QToolButton(this);
        gridLayout_->addWidget(filterBut, 0, 0);
        filterBut->setText(tr("F"));
        filterBut->setMenu(trackView_->createFilterMenu());

        // track header
        trackHeader_ = trackView_->trackHeader();
        gridLayout_->addWidget(trackHeader_, 1, 0);

        // vertical scrollbar
        vScroll_ = new QScrollBar(Qt::Vertical, this);
        gridLayout_->addWidget(vScroll_, 1, 3);

        // spacer
        spacer_ = new Spacer(Qt::Vertical, this);
        gridLayout_->addWidget(spacer_, 0, 1, 3, 1);
        spacer_->setWidgets(trackHeader_, trackView_);

        // play bar
        playBar_ = new TimeBar(this);
        playBar_->setMinimumTime(0.0);

    // --- connections ---

    // connect rulers to each other
    connect(rulerSec_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            rulerFps_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerFps_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            rulerSec_, SLOT(setViewSpace(UTIL::ViewSpace)));

    // connect rulers to trackview
    connect(rulerSec_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            trackView_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerFps_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            trackView_, SLOT(setViewSpace(UTIL::ViewSpace)));

    // connect trackview to rulers
    connect(trackView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            rulerFps_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(trackView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            rulerSec_, SLOT(setViewSpace(UTIL::ViewSpace)));

    // connect scrollbar
    connect(trackView_, SIGNAL(tracksChanged()), this, SLOT(updateVScroll_()));
    connect(vScroll_, &QScrollBar::valueChanged, [this](int v)
    {
        trackView_->setVerticalOffset(v);
    });
    connect(trackView_, &TrackView::scrollTo, [this](int y)
    {
        vScroll_->setSliderPosition(y);
    });

    // connect views to timebar
    connect(rulerSec_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            playBar_, SLOT(setViewspace(UTIL::ViewSpace)));
    connect(rulerFps_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            playBar_, SLOT(setViewspace(UTIL::ViewSpace)));
    connect(trackView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            playBar_, SLOT(setViewspace(UTIL::ViewSpace)));
    // connect timebar to outside
    connect(playBar_, SIGNAL(timeChanged(Double)),
            this, SIGNAL(sceneTimeChanged(Double)));
    // connect ruler click to scene time
    connect(rulerSec_, SIGNAL(doubleClicked(Double)),
            this, SIGNAL(sceneTimeChanged(Double)));

    // spacer to playbar-update
    connect(spacer_, SIGNAL(dragged()), this, SLOT(updateBars_()));

    // initialize viewspace
    UTIL::ViewSpace init(60, 1);
    init.setMinX(0);
    rulerSec_->setViewSpace(init, true);

    updateVScroll_();
}


void Sequencer::setSceneTime(Double time)
{
    playBar_->setTime(time);
}

void Sequencer::updateVScroll_()
{
    vScroll_->setMaximum(std::max(0, trackView_->realHeight() - trackView_->height()));
    vScroll_->setSingleStep(vScroll_->maximum() / std::max(1, trackView_->numTracks()));
}

void Sequencer::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);

    updateVScroll_();
    rulerFps_->setViewSpace(rulerFps_->viewSpace(), true);

    updateBars_();
}

void Sequencer::updatePlaybar_()
{
    playBar_->setContainingRect(
                QRect(rulerSec_->pos(),
                    QSize(rulerSec_->width(),
                          // gridLayout_->geometry().height() is sometimes zero during resize!
                          rulerFps_->geometry().bottom() - rulerSec_->geometry().top()))
                );
    // just to be sure
    playBar_->raise();
}

void Sequencer::updateLocatorBars()
{
    if (trackView_->currentObject())
    if (auto s = trackView_->currentObject()->sceneObject())
    {
        auto list = s->locators();
        for (auto i=list.begin(); i!=list.end(); ++i)
        {
            auto t = getLocatorBar_(i->id);
            t->setTime(i->time);
            t->setToolTip(QString("%1: %2").arg(i->id).arg(i->time));
        }
    }

    for (auto t : locatorBars_)
    {
        t->setContainingRect(
                    QRect(rulerSec_->pos(),
                        QSize(rulerSec_->width(),
                              // gridLayout_->geometry().height() is sometimes zero during resize!
                              rulerFps_->geometry().bottom() - rulerSec_->geometry().top()))
                    );
        t->raise();
    }
}

TimeBar* Sequencer::getLocatorBar_(const QString &name)
{
    auto i = locatorBars_.find(name);
    if (i != locatorBars_.end())
        return i.value();

    auto t = new TimeBar(this);
    t->setLocatorName("Locator");
    t->setViewspace(rulerSec_->viewSpace());
    locatorBars_.insert(name, t);

    // connect views -> timebar
    connect(rulerSec_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            t, SLOT(setViewspace(UTIL::ViewSpace)));
    connect(rulerFps_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            t, SLOT(setViewspace(UTIL::ViewSpace)));
    connect(trackView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
            t, SLOT(setViewspace(UTIL::ViewSpace)));
    // timebar -> locator time
    connect(t, &TimeBar::timeChanged, [=](Double time)
    {
        trackView_->clearSnap();
        trackView_->snapToSequences(&time);
        if (trackView_->currentObject())
        if (auto s = trackView_->currentObject()->sceneObject())
            s->setLocatorTime(name, time);
        t->setTime(time);
    });
    connect(t, &TimeBar::editingFinished, [=](){ trackView_->clearSnap(); });

    return t;
}

void Sequencer::onSequenceSelected_(Sequence * seq)
{
    // XXX not working yet
    //vScroll_->setSliderPosition(
    //            trackView_->trackY(seq->track()) );

    emit sequenceSelected(seq);
}

void Sequencer::wheelEvent(QWheelEvent * e)
{
    if (e->delta() < 0)
        vScroll_->triggerAction(QScrollBar::SliderSingleStepAdd);
    else
        vScroll_->triggerAction(QScrollBar::SliderSingleStepSub);
}

void Sequencer::setNothing()
{
    trackView_->clearTracks();
    for (auto w : locatorBars_)
        w->deleteLater();
    locatorBars_.clear();
}

void Sequencer::setCurrentObject(Object * o)
{
    MO_DEBUG_GUI("Sequencer::setCurrentObject(" << o << ")");

    trackView_->setCurrentObject(o);

    for (auto w : locatorBars_)
        w->deleteLater();
    locatorBars_.clear();
    updateLocatorBars();
}




} // namespace GUI
} // namespace MO
