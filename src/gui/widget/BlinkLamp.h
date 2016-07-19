/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/23/2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_BLINKLAMP_H
#define MOSRC_GUI_WIDGET_BLINKLAMP_H

#include <QWidget>

namespace MO {
namespace GUI {

/** A simple rectangular widget that can blink for a time */
class BlinkLamp : public QWidget
{
    Q_OBJECT
public:
    explicit BlinkLamp(QWidget *parent = 0);
    explicit BlinkLamp(const QSize& fixedSize, QWidget * parent = 0);

signals:

public slots:

    /** Lights up the widget as soon as it gets redrawn
        for the specified milliseconds. */
    void blink(int duration_ms = 100);

protected:

    void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;

private:

    QTimer * timer_;
    int doBlink_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_BLINKLAMP_H
