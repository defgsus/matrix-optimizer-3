/** @file audiolinkwindow.cpp

    @brief High-level editor for Scenes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#include <QLayout>
#include <QStatusBar>

#include "audiolinkwindow.h"
#include "audiolinkview.h"

namespace MO {
namespace GUI {



AudioLinkWindow::AudioLinkWindow(QWidget *parent)
    : QMainWindow   (parent),
      scene_        (0)
{
    setObjectName("_AudioLinkWindow");
    setWindowTitle(tr("Matrix Optimizer III"));

    setMinimumSize(640,480);

    createWidgets_();
}



void AudioLinkWindow::createWidgets_()
{
    setCentralWidget(new QWidget(this));
    centralWidget()->setObjectName("_alw_centralwidget");

    // status bar
    setStatusBar(new QStatusBar(this));

    auto lv = new QVBoxLayout(centralWidget());

        audioView_ = new AudioLinkView(this);
        lv->addWidget(audioView_);
}

void AudioLinkWindow::setScene(Scene * s)
{
    scene_ = s;

    audioView_->setScene(scene_);
}






} // namespace GUI
} // namespace MO
