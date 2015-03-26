/** @file spacer.h

    @brief Drag/spacer bar between two widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/8/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_SPACER_H
#define MOSRC_GUI_WIDGET_SPACER_H

#include <QWidget>
#include <QRect>

class QStyleOptionButton;

namespace MO {
namespace GUI {


/** Deprecated spacer between widgets.
    @deprecated because it's far not good as QDockWidget,
    although that has a serious issue with (at least x)ubuntu!
*/
class Spacer : public QWidget
{
    Q_OBJECT
public:
    explicit Spacer(Qt::Orientation, QWidget *parent = 0);
    ~Spacer();

    void setOrientation(Qt::Orientation);
    void setSpacerWidth(int);
    void setWidgets(QWidget * left_or_top, QWidget * right_or_bottom, bool adjust_left_or_top = true);

signals:

    void dragged();

public slots:

protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

    virtual void paintEvent(QPaintEvent *);

    Qt::Orientation orientation_;

    QWidget * left_, * right_;
    int width_;
    bool adjustLeft_;
    int minSize_;

    bool dragging_;
    QPoint dragStart_;
    QRect dragStartRect_;

    QStyleOptionButton * option_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_SPACER_H
