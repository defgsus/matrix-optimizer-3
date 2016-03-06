/** @file audioplugindialog.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#include <QLayout>
#include <QPushButton>

#include "audioplugindialog.h"
#include "widget/audiopluginwidget.h"
#include "audio/tool/ladspaplugin.h"
#include "io/settings.h"

namespace MO {
namespace GUI {


AudioPluginDialog::AudioPluginDialog(QWidget *parent)
    : QDialog       (parent)
{
    setObjectName("_AudioPluginDialog");
    setWindowTitle(tr("Audio plugins (currently LADSPA only)"));
    setMinimumSize(640, 480);

    settings()->restoreGeometry(this);

    createWidgets_();
}

AudioPluginDialog::~AudioPluginDialog()
{
    settings()->storeGeometry(this);
}

void AudioPluginDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        plugWidget_ = new AudioPluginWidget(this);
        lv->addWidget(plugWidget_);


        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            lh->addStretch(1);

            auto but = new QPushButton(tr("select"), this);
            connect(but, SIGNAL(clicked()), this, SLOT(accept()));
            lh->addWidget(but);

            but = new QPushButton(tr("close"), this);
            connect(but, SIGNAL(clicked()), this, SLOT(close()));
            lh->addWidget(but, -1);
}

AUDIO::LadspaPlugin * AudioPluginDialog::currentPlugin() { return plugWidget_->currentPlugin(); }

AUDIO::LadspaPlugin * AudioPluginDialog::selectPlugin(QWidget * parent)
{
    AudioPluginDialog diag(parent);

    diag.exec();

    auto plug = diag.currentPlugin();
    if (plug)
        plug->addRef("AudioPluginDialog set after select");

    return plug;
}



} // namespace GUI
} // namespace MO


#endif // #ifndef MO_DISABLE_LADSPA
