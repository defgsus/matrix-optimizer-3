/** @file transportwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/9/2014</p>
*/

#include <QLayout>
#include <QLabel>

#include "transportwidget.h"
#include "envelopewidget.h"
#include "tool/stringmanip.h"

namespace MO {
namespace GUI {

TransportWidget::TransportWidget(QWidget *parent) :
    QWidget(parent)
{
    setObjectName("_transportWidget");
    setStatusTip(tr("The transport bar"));

    auto lh = new QHBoxLayout(this);
    lh->setMargin(1);

        labelTime_ = new QLabel(this);
        auto font = labelTime_->font();
        font.setPointSize(20);
        labelTime_->setFont(font);
        labelTime_->setText(time_to_string(0.0));
        // XXX label size seems not to account for font size automatically
        labelTime_->setMinimumWidth(QFontMetrics(font).width(
                            time_to_string(60*60*1000)));

        lh->addStretch(1);

        envWidget_ = new EnvelopeWidget(this);
        lh->addWidget(envWidget_);
        envWidget_->setFixedSize(256, 48);
        envWidget_->setStatusTip(tr("Display of the microphone amplitudes"));
}

void TransportWidget::setSceneTime(Double time)
{
    /** @todo use correct fps here */
    labelTime_->setText(tr("%1\n%2s %3f")
                        .arg(time_to_string(time))
                        .arg(long(time))
                        .arg(long(time * 30)));
}


} // namespace GUI
} // namespace MO
