/** @file sequenceview.cpp

    @brief Sequence editor base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QLabel>

#include "sequenceview.h"
#include "ruler.h"


namespace MO {
namespace GUI {

SequenceView::SequenceView(QWidget *parent) :
    QWidget(parent),
    grid_           (new QGridLayout(this)),
    rulerX_         (new Ruler(this)),
    rulerY_         (new Ruler(this)),
    settings_       (0)

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
    rulerX_->setOptions(Ruler::O_DragX | Ruler::O_DrawX | Ruler::O_DrawTextX | Ruler::O_ZoomX);

    rulerY_->setFixedWidth(60);
    rulerY_->setOptions(Ruler::O_DragY | Ruler::O_DrawY | Ruler::O_DrawTextY | Ruler::O_ZoomY);

    //timelineView_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

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

    // ---- container for settings ----

    settings_ = new QScrollArea(this);
    settings_->setMinimumWidth(200);
    grid_->addWidget(settings_, 0, 0, 2, 1);

    auto w = new QWidget(settings_);
    settings_->setWidget(w);

    settingsLayout_ = new QVBoxLayout(w);
    settingsLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

    createDefaultSettings_();

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

}


void SequenceView::updateViewSpace_(const UTIL::ViewSpace & v)
{
    rulerX_->setViewSpace(v);
    rulerY_->setViewSpace(v);
}


void SequenceView::setSequenceWidget_(QWidget * w)
{
    grid_->addWidget(w, 1, 2);
}

void SequenceView::createDefaultSettings_()
{

        for (int i=0; i<10; ++i)
        {
            auto lh = new QHBoxLayout();
            settingsLayout_->addLayout(lh);
            auto l = new QLabel(settings_);
            l->setText("hello");
            lh->addWidget(l);
            auto s = new QDoubleSpinBox(settings_);
            lh->addWidget(s);
        }
}


} // namespace GUI
} // namespace MO
