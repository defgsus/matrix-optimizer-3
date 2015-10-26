/** @file objectgraphview.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#include <QDebug>
#include <QGraphicsView>
#include <QLayout>
#include <QDragEnterEvent>
#include <QMimeData>

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

    setAcceptDrops(true);

    createWidgets_();

    gview_->setScene(gscene_);
    connect(gscene_, SIGNAL(shiftView(QPointF)),
            this, SLOT(onShiftView_(QPointF)));
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

    // focus on new object
    if (root_)
    {
        // root is typically the scene and does not have an item
        auto o = root_;
        // so use the first child
        auto c = root_->childObjects();
        if (!c.isEmpty())
            o = c.first();
        // get item and ensure visible
        auto item = gscene_->itemForObject(o);
        if (item)
            gview_->ensureVisible(item);
    }
}

void ObjectGraphView::onShiftView_(const QPointF & )
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


void ObjectGraphView::keyPressEvent(QKeyEvent * e)
{
    if (Qt::Key_Plus == e->key() && (e->modifiers() & Qt::CTRL))
    {
        zoom(true);
    }
    if (Qt::Key_Minus == e->key() && (e->modifiers() & Qt::CTRL))
    {
        zoom(false);
    }
}

void ObjectGraphView::zoom(bool in)
{
    auto t = gview_->transform();
    if (in)
        t.scale(1.5, 1.5);
    else
        t.scale(.75, .75);
    gview_->setTransform(t);
}

} // namespace GUI
} // namespace MO
