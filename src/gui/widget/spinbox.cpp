/** @file spinbox.cpp

    @brief Wrapper around QSpinBox to avoid unwanted valueChanged() signal

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#include <QLayout>

#include "spinbox.h"

namespace MO {
namespace GUI {


SpinBox::SpinBox(QWidget *parent) :
    QWidget(parent),
    ignoreSignal_(false)
{
    setStatusTip(tr("Edit with keyboard, scroll with mouse-wheel or use the up/down buttons"));

    auto l = new QVBoxLayout(this);
    l->setMargin(0);
    //l->setSizeConstraint(QLayout::SetMinAndMaxSize);
    l->setSizeConstraint(QLayout::SetMaximumSize);

    spin_ = new QSpinBox(this);
    l->addWidget(spin_);

    connect(spin_, SIGNAL(valueChanged(int)), this, SLOT(internValueChanged_(int)));
}

void SpinBox::setValue(int v, bool send_signal)
{
    ignoreSignal_ = !send_signal &&
                v != spin_->value() && v >= spin_->minimum() && v <= spin_->maximum();
    spin_->setValue(v);
}

void SpinBox::internValueChanged_(int v)
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
