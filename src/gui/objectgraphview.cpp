/** @file objectgraphview.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QGraphicsScene>

#include "objectgraphview.h"
#include "util/objectgraphsettings.h"
#include "gui/item/abstractobjectitem.h"
#include "object/object.h"

namespace MO {
namespace GUI {


ObjectGraphView::ObjectGraphView(QWidget *parent)
    : QGraphicsView (parent),
      gscene_       (new QGraphicsScene(this))
{
    setScene(gscene_);

    setBackgroundBrush(ObjectGraphSettings::brushBackground());
}

void ObjectGraphView::setRootObject(Object *root)
{
    root_ = root;
    initScene_();
}

void ObjectGraphView::initScene_()
{
    gscene_->clear();

    int y = 1;
    for (Object * c : root_->childObjects())
    {
        auto item = new AbstractObjectItem(c);
        item->setGridPos(QPoint(1, y));

        gscene_->addItem(item);

        y ++;
    }
}


} // namespace GUI
} // namespace MO
