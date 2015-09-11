/** @file objectgraphview.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QDebug>
#include <QGraphicsView>
#include <QLayout>

#include "objectgraphview.h"
#include "gui/widget/iconbar.h"
#include "gui/item/abstractobjectitem.h"
#include "gui/util/objectgraphsettings.h"
#include "gui/util/objectgraphscene.h"
#include "gui/util/objectmenu.h"
#include "object/object.h"

namespace MO {
namespace GUI {


ObjectGraphView::ObjectGraphView(QWidget *parent)
    : QWidget       (parent),
      gscene_       (new ObjectGraphScene(this))
{
    setObjectName("_ObjectGraphView");

    createWidgets_();

    gview_->setScene(gscene_);
    connect(gscene_, SIGNAL(shiftView(QPointF)),
            this, SLOT(onShitView_(QPointF)));
    connect(gscene_, SIGNAL(objectSelected(MO::Object*)),
            this, SIGNAL(objectSelected(MO::Object*)));
    connect(gscene_, SIGNAL(editActionsChanged(ActionList)),
            this, SIGNAL(editActionsChanged(ActionList)));
}

void ObjectGraphView::createWidgets_()
{
    auto lv = new QVBoxLayout(this);
    lv->setMargin(0);

        // -- toolbar --

        auto bar = ObjectMenu::createObjectToolBar(Object::TG_ALL, this);
        lv->addWidget(bar);

        // -- qgraphicsview --

        gview_ = new QGraphicsView(this);
        lv->addWidget(gview_, 1);

        gview_->setBackgroundBrush(ObjectGraphSettings::brushBackground());
#if QT_VERSION >= 0x050300
        gview_->setSizeAdjustPolicy(QGraphicsView::AdjustToContents);
#endif

        gview_->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
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
        gview_->centerOn(i);
}


} // namespace GUI
} // namespace MO
