/** @file geometrymodifierwidget.cpp

    @brief Widget for GeometryModifier classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QToolButton>
#include <QIcon>

#include "geometrymodifierwidget.h"
#include "io/error.h"
#include "doublespinbox.h"
#include "spinbox.h"
#include "groupwidget.h"

#include "geom/geometrymodifierscale.h"
#include "geom/geometrymodifiertesselate.h"

namespace MO {
namespace GUI {

GeometryModifierWidget::GeometryModifierWidget(GEOM::GeometryModifier * geom, bool expanded, QWidget *parent) :
    QWidget                 (parent),
    modifier_               (geom),
    funcUpdateFromWidgets_  (0),
    funcUpdateWidgets_      (0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    createWidgets_(expanded);

    updateWidgetValues();
}


void GeometryModifierWidget::createWidgets_(bool expanded)
{
    auto l0 = new QVBoxLayout(this);
    l0->setMargin(0);
    group_ = new GroupWidget(modifier_->className(), expanded, this);
    l0->addWidget(group_);

    auto butUp = new QToolButton(this);
    group_->addHeaderWidget(butUp);
    butUp->setArrowType(Qt::UpArrow);
    butUp->setFixedSize(20,20);
    connect(butUp, &QToolButton::clicked, [=](){ emit requestUp(modifier_); });

    auto butDown = new QToolButton(this);
    group_->addHeaderWidget(butDown);
    butDown->setArrowType(Qt::DownArrow);
    butDown->setFixedSize(20,20);
    connect(butDown, &QToolButton::clicked, [=](){ emit requestDown(modifier_); });

    auto butInsert = new QToolButton(this);
    group_->addHeaderWidget(butInsert);
    butInsert->setText(".");
    butInsert->setFixedSize(20,20);
    connect(butInsert, &QToolButton::clicked, [=](){ emit requestInsertNew(modifier_); });

    auto butRemove = new QToolButton(this);
    group_->addHeaderWidget(butRemove);
    butRemove->setIcon(QIcon(":/icon/delete.png"));
    butRemove->setFixedSize(20,20);
    connect(butRemove, &QToolButton::clicked, [=](){ emit requestDelete(modifier_); });

    connect(group_, &GroupWidget::expanded, [=]()
    {
        emit expandedChange(modifier_, true);
    });
    connect(group_, &GroupWidget::collapsed, [=]()
    {
        emit expandedChange(modifier_, false);
    });


    if (auto scale = dynamic_cast<GEOM::GeometryModifierScale*>(modifier_))
    {
        auto spinall = new DoubleSpinBox(this);
        group_->addWidget(spinall);
        spinall->setStatusTip("Overall scale of the model");
        spinall->setDecimals(5);
        spinall->setSingleStep(0.1);
        spinall->setRange(0.0001, 1000000);
        connect(spinall, SIGNAL(valueChanged(double)),
                this, SLOT(updateFromWidgets_()));

        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            auto spinx = new DoubleSpinBox(this);
            group_->addWidget(spinx);
            lh->addWidget(spinx);
            spinx->setStatusTip("X-scale of the model");
            spinx->setDecimals(5);
            spinx->setSingleStep(0.1);
            spinx->setRange(0.0001, 1000000);
            connect(spinx, SIGNAL(valueChanged(double)),
                    this, SLOT(updateFromWidgets_()));

            auto spiny = new DoubleSpinBox(this);
            group_->addWidget(spiny);
            lh->addWidget(spiny);
            spiny->setStatusTip("Y-scale of the model");
            spiny->setDecimals(5);
            spiny->setSingleStep(0.1);
            spiny->setRange(0.0001, 1000000);
            connect(spiny, SIGNAL(valueChanged(double)),
                    this, SLOT(updateFromWidgets_()));

            auto spinz = new DoubleSpinBox(this);
            group_->addWidget(spinz);
            lh->addWidget(spinz);
            spinz->setStatusTip("Z-scale of the model");
            spinz->setDecimals(5);
            spinz->setSingleStep(0.1);
            spinz->setRange(0.0001, 1000000);
            connect(spinz, SIGNAL(valueChanged(double)),
                    this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            scale->setScaleAll(spinall->value());
            scale->setScaleX(spinx->value());
            scale->setScaleY(spiny->value());
            scale->setScaleZ(spinz->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinall->setValue(scale->getScaleAll());
            spinx->setValue(scale->getScaleX());
            spiny->setValue(scale->getScaleY());
            spinz->setValue(scale->getScaleZ());
        };
    }


    if (auto tess = dynamic_cast<GEOM::GeometryModifierTesselate*>(modifier_))
    {
        auto spinlevel = new SpinBox(this);
        group_->addWidget(spinlevel);
        spinlevel->setStatusTip("Order of tesselation. Be careful, this is an exponential value!");
        spinlevel->setRange(1, 10);

        funcUpdateFromWidgets_ = [=]()
        {
            tess->setTesselationLevel(spinlevel->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinlevel->setValue(tess->getTesselationLevel());
        };
    }
}

void GeometryModifierWidget::updateWidgetValues()
{
    MO_ASSERT(funcUpdateWidgets_, "no update function defined");
    if (funcUpdateWidgets_)
        funcUpdateWidgets_();
}

void GeometryModifierWidget::updateFromWidgets_()
{
    MO_ASSERT(funcUpdateFromWidgets_, "no update function defined");
    if (funcUpdateFromWidgets_)
        funcUpdateFromWidgets_();
}


} // namespace GUI
} // namespace MO
