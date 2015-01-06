/** @file timebar.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/7/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_TIMEBAR_H
#define MOSRC_GUI_WIDGET_TIMEBAR_H

#include <QWidget>
#include <QBrush>

#include "types/float.h"
#include "gui/util/viewspace.h"

namespace MO {
namespace GUI {

class TimeBar : public QWidget
{
    Q_OBJECT
public:
    explicit TimeBar(QWidget *parent = 0);

    bool isInContainingRect() const;

    bool isActive() const { return active_; }

signals:

    void timeChanged(Double time);

public slots:

    /** Use this instead of setVisible() to change visibility */
    void setActive(bool enable);

    void setContainingRect(const QRect& containing_rect);

    void setViewspace(const UTIL::ViewSpace&);

    void setTime(Double time);

    void setTimeOffset(Double offset);

    void setMinimumTime(Double time) { minTime_ = time; }
    void setMaximumTime(Double time) { maxTime_ = time; }
protected:

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);

    void update_();

    Double time_, minTime_, maxTime_, offset_;
    UTIL::ViewSpace space_;
    QRect rect_;
    QPoint dragStart_;
    Double timeStart_;
    bool active_, dragging_;

    // --- config ---

    QBrush brushBar_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_TIMEBAR_H
