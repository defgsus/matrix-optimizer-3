/** @file transportwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/9/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_TRANSPORTWIDGET_H
#define MOSRC_GUI_WIDGET_TRANSPORTWIDGET_H

#include <QWidget>

#include "types/float.h"

class QLabel;
class QToolButton;

namespace MO {
namespace GUI {

class EnvelopeWidget;

class TransportWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TransportWidget(QWidget *parent = 0);

    EnvelopeWidget * envelopeWidget() const { return envWidget_; }

signals:

    void transportBack2();
    void transportBack1();
    void transportPlay();
    void transportStop();
    void transportForward1();
    void transportForward2();

public slots:

    void setSceneTime(Double time, Double fps);
    void setPlayback(bool);

private:

    EnvelopeWidget * envWidget_;
    QLabel * labelTime_, * labelTime2_;
    QToolButton
        * butPlay,
        * butStop,
        * butBack1,
        * butBack2,
        * butFwd1,
        * butFwd2;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_TRANSPORTWIDGET_H
