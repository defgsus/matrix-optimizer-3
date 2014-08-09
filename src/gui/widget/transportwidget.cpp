/** @file transportwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/9/2014</p>
*/

#include <QLayout>

#include "transportwidget.h"
#include "envelopewidget.h"

namespace MO {
namespace GUI {

TransportWidget::TransportWidget(QWidget *parent) :
    QWidget(parent)
{
    setObjectName("_transportWidget");
    setStatusTip(tr("The transport bar"));

    auto lh = new QHBoxLayout(this);
    lh->setMargin(1);

        lh->addStretch(1);

        envWidget_ = new EnvelopeWidget(this);
        lh->addWidget(envWidget_);
        envWidget_->setFixedSize(256, 48);
        envWidget_->setStatusTip(tr("Display of the microphone amplitudes"));
}



} // namespace GUI
} // namespace MO
