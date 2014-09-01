/** @file equationdisplaydialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#include <QLayout>
#include <QToolButton>

#include "equationdisplaydialog.h"
#include "widget/equationdisplaywidget.h"
#include "widget/equationeditor.h"
#include "widget/doublespinbox.h"

namespace MO {
namespace GUI {


EquationDisplayDialog::EquationDisplayDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("_EquationDisplayDialog");
    setWindowTitle(tr("Equation tester"));

    setMinimumSize(640,480);

    createWidgets_();

    updateFromViewspace_();
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

        auto lh = new QHBoxLayout(this);
        lv->addLayout(lh);

            spinX_ = new DoubleSpinBox(this);
            lh->addWidget(spinX_);
            spinX_->setMinimum(-10000000);
            spinX_->setMaximum(10000000);
            spinX_->setSingleStep(0.5);
            spinX_->setDecimals(8);
            connect(spinX_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateViewspace_()));

            spinY_ = new DoubleSpinBox(this);
            lh->addWidget(spinY_);
            spinY_->setMinimum(-10000000);
            spinY_->setMaximum(10000000);
            spinY_->setSingleStep(0.5);
            spinY_->setDecimals(8);
            connect(spinY_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateViewspace_()));

            spinScaleX_ = new DoubleSpinBox(this);
            lh->addWidget(spinScaleX_);
            spinScaleX_->setMinimum(-10000000);
            spinScaleX_->setMaximum(10000000);
            spinScaleX_->setSingleStep(0.01);
            spinScaleX_->setDecimals(8);
            connect(spinScaleX_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateViewspace_()));

            spinScaleY_ = new DoubleSpinBox(this);
            lh->addWidget(spinScaleY_);
            spinScaleY_->setMinimum(-10000000);
            spinScaleY_->setMaximum(10000000);
            spinScaleY_->setSingleStep(0.01);
            spinScaleY_->setDecimals(8);
            connect(spinScaleY_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateViewspace_()));

            auto tbut = new QToolButton(this);
            tbut->setText(tr("reset"));
            lh->addWidget(tbut);
            connect(tbut, &QToolButton::triggered, [=]()
            {
                UTIL::ViewSpace vs(-1,-1,2,2);
                display_->setViewSpace(vs);
                updateFromViewspace_();
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

void EquationDisplayDialog::updateViewspace_()
{
    auto vs = display_->viewSpace();
    vs.setX(spinX_->value());
    vs.setY(spinY_->value());
    vs.setScaleX(spinScaleX_->value());
    vs.setScaleY(spinScaleY_->value());
    display_->setViewSpace(vs);
}

void EquationDisplayDialog::updateFromViewspace_()
{
    spinX_->setValue( display_->viewSpace().x() );
    spinY_->setValue( display_->viewSpace().y() );
    spinScaleX_->setValue( display_->viewSpace().scaleX() );
    spinScaleY_->setValue( display_->viewSpace().scaleY() );
}

} // namespace GUI
} // namespace MO
