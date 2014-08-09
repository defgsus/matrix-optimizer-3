/** @file transportwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/9/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_TRANSPORTWIDGET_H
#define MOSRC_GUI_WIDGET_TRANSPORTWIDGET_H

#include <QWidget>

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

private:

    EnvelopeWidget * envWidget_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_TRANSPORTWIDGET_H
