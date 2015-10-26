/** @file splashscreen.cpp

    @brief Start-of-application screen

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QTimer>
#include <QPixmap>

#include "splashscreen.h"


namespace MO {
namespace GUI {

namespace
{
    const QString splashFile_ =
#if defined(MO_VIOSO)
        ":/img/splashscreen_vioso.png"
#else
        ":/img/splashscreen.png"
#endif
        ;
}

SplashScreen::SplashScreen() :
    QSplashScreen(QPixmap(splashFile_), Qt::WindowStaysOnTopHint)
{
    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    timer_->setInterval(2000);
    connect(timer_, SIGNAL(timeout()), this, SLOT(close()));
}



void SplashScreen::showEvent(QShowEvent *)
{
    timer_->start();
}



} // namespace GUI
} // namespace MO
