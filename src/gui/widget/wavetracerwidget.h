/** @file wavetracerwidget.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 28.04.2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_WAVETRACERWIDGET_H
#define MOSRC_GUI_WIDGET_WAVETRACERWIDGET_H

#include <QWidget>

namespace MO {
namespace GUI {


/** Complete gui wrapper around WaveTracerShader */
class WaveTracerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaveTracerWidget(QWidget *parent = 0);
    ~WaveTracerWidget();

signals:

public slots:

    /** Restarts the tracer thread if needed */
    void updateTracer();

private slots:

    void p_tracerFinished_();
    void p_onLiveWidget_();
    void p_onWidget_();

private:

    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_WAVETRACERWIDGET_H
