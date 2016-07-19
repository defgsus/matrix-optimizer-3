/** @file clipwidgetbutton.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 14.10.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_CLIPWIDGETBUTTON_H
#define MOSRC_GUI_WIDGET_CLIPWIDGETBUTTON_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include <QPolygon>

namespace MO {
namespace GUI {

class ClipWidgetButton : public QWidget
{
    Q_OBJECT
public:

    enum State
    {
        S_OFF,
        S_TRIGGERED,
        S_ON
    };

    enum Type
    {
        T_STOP,
        T_PLAY
    };

    explicit ClipWidgetButton(Type type, QWidget *parent = 0);

    State state() const { return state_; }

public slots:

    void setState(State state);

signals:

    /** Emitted when clicked */
    void clicked();

protected:

    void updateColors_();
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void mousePressEvent(QMouseEvent*);

    Type type_;
    State state_;

    bool hasFocus_;

    QPolygon poly_;

    QPen pen_, penF_;
    QBrush brushStopped_, brushTriggered_, brushPlaying_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_CLIPWIDGETBUTTON_H
