/** @file spinbox.cpp

    @brief Wrapper around QSpinBox to avoid unwanted valueChanged() signal

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#include <QLayout>
#include <QLabel>

#include "SpinBox.h"

namespace MO {
namespace GUI {


SpinBox::SpinBox(QWidget *parent)
    : QWidget       (parent)
    , label_        (0)
    , ignoreSignal_ (false)
{
    setStatusTip(tr("Edit with keyboard, scroll with mouse-wheel or use the up/down buttons"));

    layout_ = new QHBoxLayout(this);
    layout_->setMargin(0);
    //l->setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout_->setSizeConstraint(QLayout::SetMaximumSize);

    spin_ = new QSpinBox(this);
    layout_->addWidget(spin_);
    setFocusProxy(spin_);

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

void SpinBox::setLabel(const QString & txt)
{
    if (!label_)
    {
        label_ = new QLabel(txt, this);
        label_->setAlignment(Qt::AlignRight);
        layout_->insertWidget(0, label_);
    }
    else
        label_->setText(txt);
}

} // namespace GUI
} // namespace MO
