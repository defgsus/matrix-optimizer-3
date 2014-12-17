/** @file objectgraphview.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QDebug>
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
    setObjectName("_ObjectGraphView");

    setScene(gscene_);
    connect(gscene_, SIGNAL(shiftView(QPointF)),
            this, SLOT(onShitView_(QPointF)));
    connect(gscene_, SIGNAL(objectSelected(MO::Object*)),
            this, SIGNAL(objectSelected(MO::Object*)));

    setBackgroundBrush(ObjectGraphSettings::brushBackground());
    setSizeAdjustPolicy(AdjustToContents);

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void ObjectGraphView::setGuiSettings(SceneSettings * )
{
    //gscene_->setGuiSettings(s);
}

void ObjectGraphView::setRootObject(Object *root)
{
    root_ = root;
    gscene_->setRootObject(root_);
}

void ObjectGraphView::onShitView_(const QPointF & )
{
    // XXX Nothing does work
    // rotate() even crashes, wtf..

//    translate(d.x(), d.y());
    /*auto t = QTransform();
    t.translate(d.x(), d.y());
    setTransform(t);*/
//    qDebug() << transform();
    //scrollBarWidgets();
}

void ObjectGraphView::setFocusObject(Object * o)
{
    gscene_->setFocusObject(o);
    if (auto i = gscene_->itemForObject(o))
        centerOn(i);
}


} // namespace GUI
} // namespace MO
