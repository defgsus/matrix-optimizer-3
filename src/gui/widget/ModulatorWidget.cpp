/** @file modulatorwidget.cpp

    @brief Editor for modulator settings

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include <QLayout>
#include <QLabel>

#include "ModulatorWidget.h"
#include "DoubleSpinBox.h"
#include "object/param/ModulatorFloat.h"
#include "object/Scene.h"
#include "object/util/ObjectEditor.h"
#include "io/error.h"

namespace MO {
namespace GUI {

ModulatorWidget::ModulatorWidget(QWidget *parent)
    : QWidget       (parent)
    , scene_        (0)
    , object_       (0)
    , editor_       (0)
    , modulator_    (0)
{
    auto lv = new QVBoxLayout(this);
    lv->setMargin(2);

        labelName_ = new QLabel(this);
        lv->addWidget(labelName_);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            // amplitude
            auto label = new QLabel(tr("amplitude"), this);
            lh->addWidget(label);

            spinAmplitude_ = new DoubleSpinBox(this);
            lh->addWidget(spinAmplitude_);
            spinAmplitude_->setStatusTip(tr("The amplitude of the modulation"));
            spinAmplitude_->setDecimals(5);
            spinAmplitude_->setSingleStep(0.1);
            spinAmplitude_->setRange(-99999999, 99999999);
            connect(spinAmplitude_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateFromWidgets_()));

            // time-offset
            label = new QLabel(tr("time offset"), this);
                        lh->addWidget(label);

            spinTimeOffset_ = new DoubleSpinBox(this);
            lh->addWidget(spinTimeOffset_);
            spinTimeOffset_->setStatusTip(
                        tr("The time added to look up the modulation value for tracks"));
            spinTimeOffset_->setDecimals(5);
            spinTimeOffset_->setSingleStep(0.1);
            spinTimeOffset_->setRange(-99999999, 99999999);
            connect(spinTimeOffset_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateFromWidgets_()));
}


void ModulatorWidget::setModulator(Modulator * m)
{
    if (m == 0)
    {
        modulator_ = 0;
        object_ = 0;
        scene_ = 0;
        updateWidgets_();
    }

    MO_ASSERT(m->parent(), "no parent assigned to modulator in ModulatorWidget");

    modulator_ = m;
    object_ = m->parent();
    scene_ = object_->sceneObject();
    editor_ = object_->editor();

    // foolproofish

    MO_ASSERT(scene_, "no scene assigned to modulator parent in ModulatorWidget");
    MO_ASSERT(editor_, "no editor assigned to modulator parent in ModulatorWidget");
    MO_ASSERT(modulator_->parameter(), "no parameter assigned to modulator in ModulatorWidget");

    updateWidgets_();
}

void ModulatorWidget::updateFromWidgets_()
{
    if (!modulator_)
        return;

    // XXX Not *really* needed
    //ScopedObjectChange lock(scene_, object_);

    if (ModulatorFloat * mf = dynamic_cast<ModulatorFloat*>(modulator_))
    {
        mf->setAmplitude(spinAmplitude_->value());
        mf->setTimeOffset(spinTimeOffset_->value());
        emit editor_->parameterChanged(mf->parameter());
    }
}

void ModulatorWidget::updateWidgets_()
{
    if (!modulator_)
        labelName_->setText("NULL");
    else
        labelName_->setText( modulator_->nameAutomatic() );

    ModulatorFloat * mf = dynamic_cast<ModulatorFloat*>(modulator_);

    const bool
            hasAmplitude = mf && mf->hasAmplitude(),
            hasTimeOffset = mf && mf->hasTimeOffset();

    spinAmplitude_->setEnabled(hasAmplitude);
    spinTimeOffset_->setEnabled(hasTimeOffset);

    if (mf)
    {
        spinAmplitude_->setValue(mf->amplitude());
        spinTimeOffset_->setValue(mf->timeOffset());
    }
}


} // namespace GUI
} // namespace MO
