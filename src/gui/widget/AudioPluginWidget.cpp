/** @file audiopluginwidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QTextBrowser>

#include "AudioPluginWidget.h"
#include "io/Files.h"
#include "io/LadspaLoader.h"
#include "audio/tool/LadspaPlugin.h"

namespace MO {
namespace GUI {

struct AudioPluginWidget::Private
{
    Private(AudioPluginWidget*w)
        : widget        (w)
        , curPlug       (0)
    {

    }

    void createWidgets();
    void scanDir(const QString& path);
    void selectPlugin(int index);

    AudioPluginWidget * widget;

    QLabel * labelDir;
    QListWidget * listPlugs;
    QTextBrowser * textPlug;
    IO::LadspaLoader loader;
    QList<AUDIO::LadspaPlugin*> plugs;
    AUDIO::LadspaPlugin * curPlug;
};

AudioPluginWidget::AudioPluginWidget(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    p_->createWidgets();

    QString dir = IO::Files::directory(IO::FT_LADSPA);
    if (!dir.isEmpty())
        p_->scanDir(dir);
}

AudioPluginWidget::~AudioPluginWidget()
{
    delete p_;
}

AUDIO::LadspaPlugin * AudioPluginWidget::currentPlugin() const
{
    return p_->curPlug;
}

void AudioPluginWidget::Private::createWidgets()
{
    auto lv0 = new QVBoxLayout(widget);
    lv0->setMargin(0);

        // input directory

        auto lh = new QHBoxLayout;
        lv0->addLayout(lh);

            labelDir = new QLabel(widget);
            lh->addWidget(labelDir, 1);

            auto but = new QPushButton(tr("..."), widget);
            connect(but, SIGNAL(clicked()), widget, SLOT(chooseDirectory()));
            lh->addWidget(but);

        lh = new QHBoxLayout;
        lv0->addLayout(lh);

            listPlugs = new QListWidget(widget);
            connect(listPlugs, &QListWidget::currentRowChanged, [=](int index)
            {
                selectPlugin(index);
            });
            lh->addWidget(listPlugs);

            textPlug = new QTextBrowser(widget);
            lh->addWidget(textPlug);
}

void AudioPluginWidget::chooseDirectory()
{
    QString fn = IO::Files::getDirectory(IO::FT_LADSPA, this, true,
                                         tr("Select LADSPA folder"));
    if (!fn.isEmpty())
        p_->scanDir(fn);
}

void AudioPluginWidget::Private::scanDir(const QString &path)
{
    labelDir->setText(path);
    listPlugs->clear();
    plugs.clear();
    loader.clear();
    curPlug = 0;

    size_t num = loader.loadDirectory(path);
    if (!num)
    {
        QMessageBox::information(widget, tr("LADSPA folder"),
                                 tr("Could not find any LADSPA plugins in folder\n'%1'")
                                 .arg(path));
        return;
    }

    // populate list widget

    // copy list (to easy use of index from listPlugs)
    plugs = loader.getPlugins();
    for (AUDIO::LadspaPlugin * plug : plugs)
    {
        listPlugs->addItem(plug->name());
    }
}

void AudioPluginWidget::Private::selectPlugin(int index)
{
    textPlug->clear();

    if (index < 0 || index >= plugs.size())
        return;

    curPlug = plugs[index];
    textPlug->setPlainText(curPlug->infoString());
}

} // namespace GUI
} // namespace MO



#endif // #ifndef MO_DISABLE_LADSPA
