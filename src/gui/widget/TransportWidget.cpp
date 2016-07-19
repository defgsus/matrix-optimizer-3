/** @file transportwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/9/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QToolButton>

#include "TransportWidget.h"
#include "EnvelopeWidget.h"
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

        auto lv = new QVBoxLayout();
        lv->setMargin(0);
        lh->addLayout(lv);

            labelTime_ = new QLabel(this);
            auto font = labelTime_->font();
            font.setPointSize(20);
            labelTime_->setFont(font);
            labelTime_->setMinimumWidth(QFontMetrics(font).width(
                                time_to_string(60*60*1000, true)));
            lv->addWidget(labelTime_);

            labelTime2_ = new QLabel(this);
            font.setPointSize(14);
            labelTime2_->setFont(font);
            lv->addWidget(labelTime2_);

            lv->addStretch();

        lh->addStretch(1);

        QToolButton * but;
        butBack2 = but = new QToolButton(this);
        but->setIcon(QIcon(":/icon/trans_back2.png"));
        lh->addWidget(but);
        connect(but, SIGNAL(clicked(bool)), this, SIGNAL(transportBack2()));

        butBack1 = but = new QToolButton(this);
        but->setIcon(QIcon(":/icon/trans_back1.png"));
        lh->addWidget(but);
        connect(but, SIGNAL(clicked(bool)), this, SIGNAL(transportBack1()));

        butPlay = but = new QToolButton(this);
        but->setIcon(QIcon(":/icon/trans_play.png"));
        lh->addWidget(but);
        connect(but, SIGNAL(clicked(bool)), this, SIGNAL(transportPlay()));

        butStop = but = new QToolButton(this);
        but->setIcon(QIcon(":/icon/trans_stop.png"));
        lh->addWidget(but);
        connect(but, SIGNAL(clicked(bool)), this, SIGNAL(transportStop()));

        butFwd1 = but = new QToolButton(this);
        but->setIcon(QIcon(":/icon/trans_fwd1.png"));
        lh->addWidget(but);
        connect(but, SIGNAL(clicked(bool)), this, SIGNAL(transportForward1()));

        butFwd2 = but = new QToolButton(this);
        but->setIcon(QIcon(":/icon/trans_fwd2.png"));
        lh->addWidget(but);
        connect(but, SIGNAL(clicked(bool)), this, SIGNAL(transportForward2()));


        lh->addStretch(1);

        envWidget_ = new EnvelopeWidget(this);
        lh->addWidget(envWidget_);
        envWidget_->setFixedSize(256, 48);
        envWidget_->setStatusTip(tr("Display of the microphone amplitudes"));


    setSceneTime(0., 0.);
    setPlayback(false);
}

void TransportWidget::setSceneTime(Double time, Double fps)
{
    /** @todo use correct fps here */
    labelTime_->setText(time_to_string(time, true));
    labelTime2_->setText(tr("%2s %3f (%4fps)")
                        .arg(long(time))
                        .arg(long(time * 60.))
                        .arg(int(fps))
                        );
}

void TransportWidget::setPlayback(bool p)
{
    butPlay->setDown(p);
    butStop->setDown(!p);
}

} // namespace GUI
} // namespace MO
