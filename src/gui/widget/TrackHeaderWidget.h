/** @file trackheaderwidget.h

    @brief Widget per track for TrackHeader

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_TRACKHEADERWIDGET_H
#define MOSRC_GUI_WIDGET_TRACKHEADERWIDGET_H

#include <QWidget>

class QHBoxLayout;
class QLabel;

namespace MO {
class Track;
namespace GUI {

class TrackHeaderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrackHeaderWidget(Track * track, QWidget *parent = 0);

    Track * track() const { return track_; }

signals:

    void heightChange(int);

public slots:

protected:

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    Track * track_;

    QHBoxLayout * layout_;

    // ---- interaction ----

    QPoint dragStart_;
    int dragStartHeight_;
    bool dragging_, onEdge_;
};


} // namespace GUI
} // namespace MO



#endif // MOSRC_GUI_WIDGET_TRACKHEADERWIDGET_H
