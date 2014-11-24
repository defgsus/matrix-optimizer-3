/** @file objectgraphview.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QGraphicsScene>

#include "objectgraphview.h"
#include "util/objectgraphsettings.h"
#include "gui/util/objectgraphscene.h"
#include "gui/item/abstractobjectitem.h"
#include "object/object.h"

namespace MO {
namespace GUI {


ObjectGraphView::ObjectGraphView(QWidget *parent)
    : QGraphicsView (parent),
      gscene_       (new ObjectGraphScene(this))
{
    setScene(gscene_);

    setBackgroundBrush(ObjectGraphSettings::brushBackground());
}

void ObjectGraphView::setRootObject(Object *root)
{
    root_ = root;
    gscene_->setRootObject(root_);
}


} // namespace GUI
} // namespace MO
