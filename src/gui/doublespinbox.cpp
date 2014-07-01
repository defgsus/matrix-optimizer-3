/** @file doublespinbox.cpp

    @brief Wrapper around QDoubleSpinBox to avoid unwanted valueChanged signal

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#include <QLayout>

#include "doublespinbox.h"

namespace MO {
namespace GUI {


DoubleSpinBox::DoubleSpinBox(QWidget *parent) :
    QWidget(parent),
    ignoreSignal_(false)
{
    auto l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSizeConstraint(QLayout::SetMinAndMaxSize);

    spin_ = new QDoubleSpinBox(this);
    l->addWidget(spin_);

    connect(spin_, SIGNAL(valueChanged(double)), this, SLOT(internValueChanged_(double)));
}

void DoubleSpinBox::setValue(double v, bool send_signal)
{
    ignoreSignal_ = !send_signal && v != spin_->value();
    spin_->setValue(v);
}

void DoubleSpinBox::internValueChanged_(double v)
{
    if (ignoreSignal_)
    {
        ignoreSignal_ = false;
    }
    else
        emit valueChanged(v);
}


} // namespace GUI
} // namespace MO
