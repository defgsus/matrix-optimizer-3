/** @file equationdisplaydialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#include <QLayout>

#include "equationdisplaydialog.h"
#include "widget/equationdisplaywidget.h"

namespace MO {
namespace GUI {


EquationDisplayDialog::EquationDisplayDialog(QWidget *parent) :
    QDialog(parent)
{
    setMinimumSize(640,480);

    createWidgets_();
}

void EquationDisplayDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

    display_ = new EquationDisplayWidget(this);
    lv->addWidget(display_);
}


} // namespace GUI
} // namespace MO
