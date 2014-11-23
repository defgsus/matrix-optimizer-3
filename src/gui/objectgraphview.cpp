/** @file objectgraphview.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QGraphicsScene>

#include "objectgraphview.h"


namespace MO {
namespace GUI {


ObjectGraphView::ObjectGraphView(QWidget *parent)
    : QGraphicsView (parent),
      gscene_       (new QGraphicsScene(this))
{

}




} // namespace GUI
} // namespace MO
