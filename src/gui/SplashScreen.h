/** @file splashscreen.h

    @brief Start-of-application screen

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#ifndef MOSRC_GUI_SPLASHSCREEN_H
#define MOSRC_GUI_SPLASHSCREEN_H

#include <QSplashScreen>

class QTimer;

namespace MO {
namespace GUI {

class SplashScreen : public QSplashScreen
{
    Q_OBJECT
public:
    explicit SplashScreen();

signals:

public slots:

protected:
    void showEvent(QShowEvent *);

    QTimer * timer_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SPLASHSCREEN_H
