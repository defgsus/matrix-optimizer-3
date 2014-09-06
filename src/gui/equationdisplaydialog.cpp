/** @file equationdisplaydialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#include <QLayout>
#include <QToolButton>
#include <QComboBox>
#include <QThread>

#include "equationdisplaydialog.h"
#include "widget/equationdisplaywidget.h"
#include "widget/equationeditor.h"
#include "widget/doublespinbox.h"
#include "io/settings.h"

namespace MO {
namespace GUI {


EquationDisplayDialog::EquationDisplayDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("_EquationDisplayDialog");
    setWindowTitle(tr("Equation tester"));

    setMinimumSize(640,800);

    createWidgets_();

    loadSettings_();
}

EquationDisplayDialog::~EquationDisplayDialog()
{
    saveSettings_();
}

void EquationDisplayDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        // display

        display_ = new EquationDisplayWidget(this);
        display_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lv->addWidget(display_, 3);
        connect(display_, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)),
                this, SLOT(updateFromViewspace_()));

        // viewspace

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            spinX0_ = new DoubleSpinBox(this);
            lh->addWidget(spinX0_);
            spinX0_->setMinimum(-10000000);
            spinX0_->setMaximum(10000000);
            spinX0_->setSingleStep(0.5);
            spinX0_->setDecimals(8);
            connect(spinX0_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateViewspace_()));

            spinY0_ = new DoubleSpinBox(this);
            lh->addWidget(spinY0_);
            spinY0_->setMinimum(-10000000);
            spinY0_->setMaximum(10000000);
            spinY0_->setSingleStep(0.5);
            spinY0_->setDecimals(8);
            connect(spinY0_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateViewspace_()));

            spinX1_ = new DoubleSpinBox(this);
            lh->addWidget(spinX1_);
            spinX1_->setMinimum(-10000000);
            spinX1_->setMaximum(10000000);
            spinX1_->setSingleStep(0.5);
            spinX1_->setDecimals(8);
            connect(spinX1_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateViewspace_()));

            spinY1_ = new DoubleSpinBox(this);
            lh->addWidget(spinY1_);
            spinY1_->setMinimum(-10000000);
            spinY1_->setMaximum(10000000);
            spinY1_->setSingleStep(0.5);
            spinY1_->setDecimals(8);
            connect(spinY1_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateViewspace_()));

            auto tbut = new QToolButton(this);
            tbut->setText(tr("reset"));
            lh->addWidget(tbut);
            connect(tbut, &QToolButton::clicked, [=]()
            {
                display_->resetViewSpace();
                updateFromViewspace_();
            });

        // paint mode

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            comboMode_ = new QComboBox(this);
            lh->addWidget(comboMode_);

            comboMode_->addItem(tr("f(x)"), EquationDisplayWidget::PM_F_OF_X);
            comboMode_->addItem(tr("f(x,y)"), EquationDisplayWidget::PM_F_OF_XY);
            comboMode_->addItem(tr("2D integer number"), EquationDisplayWidget::PM_2D_INTEGER_NUM);
            comboMode_->addItem(tr("2D integer square"), EquationDisplayWidget::PM_2D_INTEGER_SQUARE);

            connect(comboMode_, static_cast<void(QComboBox::*)(int)>
                                (&QComboBox::currentIndexChanged), [=]()
            {
                display_->setPaintMode(
                            (EquationDisplayWidget::PaintMode)
                                comboMode_->itemData(comboMode_->currentIndex()).toInt());
            });

        // editor

        editor_ = new EquationEditor(this);
        lv->addWidget(editor_, 1);
        editor_->setParser(display_->parser());
        editor_->setPlainText(display_->equation());

        connect(editor_, &EquationEditor::equationChanged, [=]()
        {
            display_->setEquation( editor_->toPlainText() );
        });

}

void EquationDisplayDialog::closeEvent(QCloseEvent * e)
{
    //saveSettings_();
    QDialog::closeEvent(e);
}

void EquationDisplayDialog::updateViewspace_()
{
    auto vs = display_->viewSpace();
    vs.setX(spinX0_->value());
    vs.setY(spinY0_->value());
    vs.setScaleX(spinX1_->value() - spinX0_->value());
    vs.setScaleY(spinY1_->value() - spinY0_->value());
    display_->setViewSpace(vs);
}

void EquationDisplayDialog::updateFromViewspace_()
{
    spinX0_->setValue( display_->viewSpace().x() );
    spinY0_->setValue( display_->viewSpace().y() );
    spinX1_->setValue( display_->viewSpace().x() + display_->viewSpace().scaleX() );
    spinY1_->setValue( display_->viewSpace().y() + display_->viewSpace().scaleY() );
}


void EquationDisplayDialog::updateModeBox_()
{
    for (int i=0; i<comboMode_->count(); ++i)
        if (comboMode_->itemData(i).toInt() == display_->paintMode())
        {
            comboMode_->setCurrentIndex(i);
        }
}

void EquationDisplayDialog::saveSettings_()
{
    settings->setValue("EquEdit/equation", display_->equation());
    settings->setValue("EquEdit/paintmode", display_->paintMode());
    settings->setValue("EquEdit/x0", spinX0_->value());
    settings->setValue("EquEdit/x1", spinX1_->value());
    settings->setValue("EquEdit/y0", spinY0_->value());
    settings->setValue("EquEdit/y1", spinY1_->value());
}

void EquationDisplayDialog::loadSettings_()
{
    const QString equ = settings->getValue("EquEdit/equation").toString();
    editor_->setPlainText( equ );

    auto pm = (EquationDisplayWidget::PaintMode)
                settings->getValue("EquEdit/paintmode").toInt();
    display_->setPaintMode( pm );
    updateModeBox_();

    spinX0_->setValue( settings->getValue("EquEdit/x0").toDouble() );
    spinX1_->setValue( settings->getValue("EquEdit/x1").toDouble() );
    spinY0_->setValue( settings->getValue("EquEdit/y0").toDouble() );
    spinY1_->setValue( settings->getValue("EquEdit/y1").toDouble() );
    updateViewspace_();
}

} // namespace GUI
} // namespace MO
