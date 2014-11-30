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

public slots:

    void setSceneTime(Double time);

private:

    EnvelopeWidget * envWidget_;
    QLabel * labelTime_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_TRANSPORTWIDGET_H
