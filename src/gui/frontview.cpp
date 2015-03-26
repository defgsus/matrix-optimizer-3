/** @file frontview.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#include <QLayout>
#include <QGraphicsView>

#include "frontview.h"
#include "widget/presetswidget.h"
#include "util/frontscene.h"


namespace MO {
namespace GUI {



FrontView::FrontView(QWidget *parent)
    : QWidget       (parent)
    , gscene_       (0)
{
    setObjectName("_FrontView");

    createWidgets_();
}

void FrontView::createWidgets_()
{
    auto lv = new QVBoxLayout(this);
    lv->setMargin(0);

        // presets

        presets_ = new PresetsWidget(this);
        lv->addWidget(presets_);


        // QGraphicsView
        gview_ = new QGraphicsView(this);
        lv->addWidget(gview_, 1);

    #if QT_VERSION >= 0x050300
        gview_->setSizeAdjustPolicy(gview_->AdjustToContents);
    #endif
        gview_->setTransformationAnchor(gview_->AnchorViewCenter);

        gview_->setRubberBandSelectionMode(Qt::IntersectsItemShape);
        gview_->setMouseTracking(true);

        gview_->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void FrontView::setFrontScene(FrontScene * s)
{
    bool changed = gscene_ != s;
    gview_->setScene(gscene_ = s);

    if (changed)
    {
        onEditModeChange_(gscene_->isEditMode());
        connect(gscene_, SIGNAL(editModeChanged(bool)),
                this, SLOT(onEditModeChange_(bool)));
        connect(gscene_, &FrontScene::presetsChanged, [=]()
        {
            presets_->setPresets(gscene_->presets());
        });

        presets_->setPresets(gscene_->presets());

        connect(presets_, SIGNAL(presetSaveRequest(QString)),
                gscene_, SLOT(storePreset(QString)));
        connect(presets_, SIGNAL(presetLoadRequest(QString)),
                gscene_, SLOT(loadPreset(QString)));

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
        gview_->setDragMode(gview_->RubberBandDrag);
    else
        gview_->setDragMode(gview_->ScrollHandDrag);
}

} // namespace GUI
} // namespace MO
