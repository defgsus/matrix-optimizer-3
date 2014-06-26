/** @file timeline1drulerview.cpp

    @brief Timeline1DView with rulers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/26/2014</p>
*/

#include <QLayout>

#include "timeline1drulerview.h"
#include "timeline1dview.h"
#include "ruler.h"

namespace MO {
namespace GUI {


Timeline1DRulerView::Timeline1DRulerView(MATH::Timeline1D *timeline, QWidget *parent)
    :   QWidget(parent),
        timelineView_   (new Timeline1DView(timeline, this)),
        rulerX_         (new Ruler(this)),
        rulerY_         (new Ruler(this)),
        layout_         (new QGridLayout(this))

{
    rulerX_->setFixedHeight(40);
    layout_->addWidget(rulerX_, 0, 1);
    rulerX_->setOptions(Ruler::O_DragX | Ruler::O_DrawX | Ruler::O_DrawTextX | Ruler::O_ZoomX);

    rulerY_->setFixedWidth(60);
    layout_->addWidget(rulerY_, 1, 0);
    rulerY_->setOptions(Ruler::O_DragY | Ruler::O_DrawY | Ruler::O_DrawTextY | Ruler::O_ZoomY);

    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), timelineView_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX_, SLOT(setViewSpace(UTIL::ViewSpace)));

    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), timelineView_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(timelineView_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY_, SLOT(setViewSpace(UTIL::ViewSpace)));

    connect(rulerX_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY_, SLOT(setViewSpace(UTIL::ViewSpace)));
    connect(rulerY_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX_, SLOT(setViewSpace(UTIL::ViewSpace)));

    rulerX_->setViewSpace(rulerX_->viewSpace(), true);

}


} // namespace GUI
} // namespace MO
