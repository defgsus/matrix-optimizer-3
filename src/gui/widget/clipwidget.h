/** @file clipwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_CLIPWIDGET_H
#define MOSRC_GUI_WIDGET_CLIPWIDGET_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include <QStaticText>

namespace MO {
class Clip;
namespace GUI {

class ClipView;

class ClipWidget : public QWidget
{
    Q_OBJECT
public:

    enum Type
    {
        T_COLUMN,
        T_ROW,
        T_CLIP
    };

    explicit ClipWidget(Type type, Clip * clip, ClipView *parent = 0);

    Type type() const { return type_; }

    const QString& name() const { return name_; }

    Clip * clip() const { return clip_; }

    uint posX() const { return x_; }
    uint posY() const { return y_; }
    void setPos(uint x, uint y) { x_=x; y_=y; }

public slots:

    /** Sets the name to display */
    void setName(const QString&);

    /** Sets a new or no Clip */
    void setClip(Clip *);

signals:

    /** Emitted when clicked */
    void clicked(ClipWidget*, Qt::MouseButtons, Qt::KeyboardModifiers);
    void moved(ClipWidget*, const QPoint&, Qt::MouseButtons, Qt::KeyboardModifiers);
    void released(ClipWidget*, Qt::MouseButtons, Qt::KeyboardModifiers);

protected:

    void updateColors_();
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

    ClipView * view_;
    Clip * clip_;

    Type type_;

    QString name_;
    QStaticText nameText_;

    bool hasFocus_;

    uint x_, y_;

    QPen penOutline_, penOutlineS_, penText_;
    QBrush brushBody_, brushBodyH_, brushBodyS_, brushBodySH_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_CLIPWIDGET_H
