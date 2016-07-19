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
#include "gui/util/ViewSpace.h"

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

    /** During dragging */
    void timeChanged(Double time);
    /** On mouse-release after drag */
    void editingFinished();

public slots:

    /** Use this instead of setVisible() to change visibility */
    void setActive(bool enable);

    void setContainingRect(const QRect& containing_rect);

    void setViewspace(const UTIL::ViewSpace&);

    void setTime(Double time);

    void setTimeOffset(Double offset);

    void setMinimumTime(Double time) { minTime_ = time; }
    void setMaximumTime(Double time) { maxTime_ = time; }

    void setLocatorName(const QString& n) { locatorName_ = n; update(); }

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
    QString locatorName_;

    // --- config ---

    QBrush brushBar_, brushLocatorBar_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_TIMEBAR_H
