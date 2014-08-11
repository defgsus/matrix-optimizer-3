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
    setStatusTip(tr("Edit with keyboard, scroll with mouse-wheel or use the up/down buttons"));

    auto l = new QVBoxLayout(this);
    l->setMargin(0);
    //l->setSizeConstraint(QLayout::SetMinAndMaxSize);
    l->setSizeConstraint(QLayout::SetMaximumSize);

    spin_ = new QDoubleSpinBox(this);
    l->addWidget(spin_);

    connect(spin_, SIGNAL(valueChanged(double)), this, SLOT(internValueChanged_(double)));
}

void DoubleSpinBox::setValue(double v, bool send_signal)
{
    ignoreSignal_ = !send_signal &&
            v != spin_->value() && v >= spin_->minimum() && v <= spin_->maximum();
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
