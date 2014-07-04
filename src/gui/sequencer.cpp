/** @file sequencer.cpp

    @brief Sequencer using TrackView

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#include <QGridLayout>
#include <QScrollBar>

#include "sequencer.h"
#include "trackheader.h"
#include "trackview.h"
#include "ruler.h"

namespace MO {
namespace GUI {


Sequencer::Sequencer(QWidget *parent) :
    QWidget(parent)
{
    createWidgets_();
}



void Sequencer::createWidgets_()
{
    gridLayout_ = new QGridLayout(this);
    gridLayout_->setMargin(2);

        // ruler seconds
        rulerSec_ = new Ruler(this);
        gridLayout_->addWidget(rulerSec_, 0, 1);
        rulerSec_->setOptions(Ruler::O_EnableAllX);
        rulerSec_->setFixedHeight(36);

        // ruler fps
        rulerFps_ = new Ruler(this);
        gridLayout_->addWidget(rulerFps_, 2, 1);
        rulerFps_->setOptions(Ruler::O_EnableAllX);
        rulerFps_->setFixedHeight(36);

        // track view
        trackView_ = new TrackView(this);
        gridLayout_->addWidget(trackView_, 1, 1);

        // track header
        trackHeader_ = trackView_->trackHeader();
        gridLayout_->addWidget(trackHeader_, 1, 0);

        // vertical scrollbar
        vScroll_ = new QScrollBar(Qt::Vertical, this);
        gridLayout_->addWidget(vScroll_, 1, 2);

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
}



} // namespace GUI
} // namespace MO
