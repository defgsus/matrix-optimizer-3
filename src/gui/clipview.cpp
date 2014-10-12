/** @file clipview.cpp

    @brief Clip object view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include "clipview.h"

namespace MO {
namespace GUI {



ClipView::ClipView(QWidget * parent)
    : QGraphicsView (parent),
      scene_        (new QGraphicsScene(this))
{
}




} // namespace GUI
} // namespace MO
