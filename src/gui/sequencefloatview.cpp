/** @file sequencefloatview.cpp

    @brief float sequence editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include "sequencefloatview.h"
#include "timeline1dview.h"
#include "ruler.h"
#include "math/timeline1d.h"

namespace MO {
namespace GUI {


SequenceFloatView::SequenceFloatView(QWidget *parent) :
    SequenceView(parent)
{
    auto tl = new MATH::Timeline1D;
    for (int i=0; i<20; ++i)
        tl->add((Double)rand()/RAND_MAX * 10.0, (Double)rand()/RAND_MAX, MATH::Timeline1D::Point::SYMMETRIC);
    tl->setAutoDerivative();

    timeline_ = new Timeline1DView(tl, this);
    timeline_->setGridOptions(Ruler::O_DrawX | Ruler::O_DrawY);

    setSequenceWidget_(timeline_);
}


void SequenceFloatView::setViewSpace(const UTIL::ViewSpace & v)
{
    timeline_->setViewSpace(v);
    updateViewSpace_(v);
}

} // namespace GUI
} // namespace MO
