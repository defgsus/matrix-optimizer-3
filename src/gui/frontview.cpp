/** @file frontview.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include "frontview.h"
#include "util/frontscene.h"

namespace MO {
namespace GUI {



FrontView::FrontView(QWidget *parent)
    : QGraphicsView (parent)
    , gscene_       (new FrontScene(this))
{
  setObjectName("_FrontView");

  setScene(gscene_);

  //setBackgroundBrush(ObjectGraphSettings::brushBackground());
#if QT_VERSION >= 0x050300
  setSizeAdjustPolicy(AdjustToContents);
#endif

  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void FrontView::setRootObject(Object *root)
{
    gscene_->setRootObject(root);
}


void FrontView::setFocusObject(Object * o)
{
    //gscene_->setFocusObject(o);
    //if (auto i = gscene_->itemForObject(o))
    //    centerOn(i);
}

} // namespace GUI
} // namespace MO
