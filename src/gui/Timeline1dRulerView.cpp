/** @file timeline1drulerview.cpp

    @brief Timeline1DView with rulers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/26/2014</p>
*/


#include <QDebug>
#include <QLayout>

#include "Timeline1dRulerView.h"
#include "Timeline1dView.h"
#include "Ruler.h"

namespace MO {
namespace GUI {


Timeline1DRulerView::Timeline1DRulerView(MATH::Timeline1d *timeline, QWidget *parent)
    :   QWidget(parent),
        timelineView_   (new Timeline1DView(timeline, this)),
        rulerX_         (new Ruler(this)),
        rulerY_         (new Ruler(this)),
        layout_         (new QGridLayout(this))

{
    layout_->setMargin(1);
    layout_->setContentsMargins(1,1,1,1);
    layout_->setSpacing(2);

    layout_->addWidget(timelineView_, 1, 1);
    layout_->addWidget(rulerX_, 0, 1);
    layout_->addWidget(rulerY_, 1, 0);

    rulerX_->setFixedHeight(40);
    rulerX_->setOptions(Ruler::O_DragX | Ruler::O_DrawX | Ruler::O_DrawTextX | Ruler::O_ZoomX);

    rulerY_->setFixedWidth(60);
    rulerY_->setOptions(Ruler::O_DragY | Ruler::O_DrawY | Ruler::O_DrawTextY | Ruler::O_ZoomY);

    timelineView_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    // connect rulers to timeline and vice versa

    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), timelineView_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX_, SLOT(setViewSpace(UTIL::ViewSpace)));

    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), timelineView_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY_, SLOT(setViewSpace(UTIL::ViewSpace)));

    // connect rulers to each other

    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX_, SLOT(setViewSpace(UTIL::ViewSpace)));

    // pass viewSpaceChanged to this classes signal
    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));
    connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), this, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)));

    // connect fit-request from ruler to timeline's fit-to-view
    connect(rulerX_, &Ruler::fitRequest, [=]()
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

    // forward timelineview's signals
    connect(timelineView_, SIGNAL(timelineChanged()), this, SIGNAL(timelineChanged()));

    // update rulers
    timelineView_->setViewSpace(timelineView_->viewSpace(), true);
}

MATH::Timeline1d * Timeline1DRulerView::timeline() const
{
    return timelineView_->timeline();
}

void Timeline1DRulerView::setTimeline(MATH::Timeline1d *timeline)
{
    timelineView_->setTimeline(timeline);
}

const UTIL::ViewSpace& Timeline1DRulerView::viewSpace() const
{
    return timelineView_->viewSpace();
}

void Timeline1DRulerView::setGridOptions(int options)
{
    timelineView_->setGridOptions(options);
}

void Timeline1DRulerView::setOptions(int options)
{
    timelineView_->setOptions(options);

    // adjust rulers
    rulerX_->setOptions(
                (Ruler::O_DragX * ((options & Timeline1DView::O_MoveViewX) != 0))
            |   (Ruler::O_ZoomX * ((options & Timeline1DView::O_ZoomViewX) != 0))
                // keep all other options
            |   ((~(Ruler::O_DragX | Ruler::O_ZoomX)) & rulerX_->options())
                         );

    rulerY_->setOptions(
                (Ruler::O_DragY * ((options & Timeline1DView::O_MoveViewY) != 0))
            |   (Ruler::O_ZoomY * ((options & Timeline1DView::O_ZoomViewY) != 0))
            |   ((~(Ruler::O_DragY | Ruler::O_ZoomY)) & rulerY_->options())
                         );
}

void Timeline1DRulerView::setViewSpace(const UTIL::ViewSpace & v, bool send_signal)
{
    timelineView_->setViewSpace(v);
    rulerX_->setViewSpace(v);
    rulerY_->setViewSpace(v);
    if (send_signal)
        emit viewSpaceChanged(v);
}

void Timeline1DRulerView::fitToView(bool fitX, bool fitY, int marginInPixels)
{
    timelineView_->fitToView(fitX, fitY, marginInPixels);
}

void Timeline1DRulerView::fitSelectionToView(bool fitX, bool fitY, int marginInPixels)
{
    timelineView_->fitSelectionToView(fitX, fitY, marginInPixels);
}

void Timeline1DRulerView::unselect()
{
    timelineView_->unselect();
}


} // namespace GUI
} // namespace MO
