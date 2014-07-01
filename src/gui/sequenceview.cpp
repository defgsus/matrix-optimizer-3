/** @file sequenceview.cpp

    @brief Sequence editor base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QGridLayout>
#include <QDoubleSpinBox>

#include "sequenceview.h"
#include "ruler.h"


namespace MO {
namespace GUI {

SequenceView::SequenceView(QWidget *parent) :
    QWidget(parent),
    grid_           (new QGridLayout(this)),
    rulerX_         (new Ruler(this)),
    rulerY_         (new Ruler(this))

{
    grid_->setMargin(1);
    grid_->setContentsMargins(1,1,1,1);
    grid_->setSpacing(2);

    //grid_->addWidget(timelineView_, 1, 1);
    grid_->addWidget(rulerX_, 0, 1);
    grid_->addWidget(rulerY_, 1, 0);

    // set ruler properties

    rulerX_->setFixedHeight(40);
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
    grid_->addWidget(1, 1);
}

} // namespace GUI
} // namespace MO
