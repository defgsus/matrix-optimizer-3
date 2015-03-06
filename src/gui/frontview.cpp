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
    , gscene_       (0)
{
    setObjectName("_FrontView");

#if QT_VERSION >= 0x050300
    setSizeAdjustPolicy(AdjustToContents);
#endif

    setRubberBandSelectionMode(Qt::IntersectsItemShape);
    setMouseTracking(true);

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

}

void FrontView::setFrontScene(FrontScene * s)
{
    bool changed = gscene_ != s;
    setScene(gscene_ = s);

    if (changed)
    {
        onEditModeChange_(gscene_->isEditMode());
        connect(gscene_, SIGNAL(editModeChanged(bool)),
                this, SLOT(onEditModeChange_(bool)));
        // install default actions
        //addActions(gscene_->createDefaultActions());
    }
}

void FrontView::setFocusObject(Object * o)
{
    //gscene_->setFocusObject(o);
    //if (auto i = gscene_->itemForObject(o))
    //    centerOn(i);
}

void FrontView::onEditModeChange_(bool e)
{
    if (e)
        setDragMode(RubberBandDrag);
    else
        setDragMode(ScrollHandDrag);
}

} // namespace GUI
} // namespace MO
