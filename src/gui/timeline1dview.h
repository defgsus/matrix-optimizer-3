/** @file

    @brief widget for Timeline1D

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/24</p>
*/

#ifndef MOSRC_GUI_TIMELINE1DVIEW_H
#define MOSRC_GUI_TIMELINE1DVIEW_H

#include <QWidget>

#include "math/timeline1d.h"

namespace MO {
namespace GUI {

class Timeline1DView : public QWidget
{
    Q_OBJECT
public:
    explicit Timeline1DView(Timeline1D * timeline = 0, QWidget *parent = 0);

    /** Assigns a new (or no) Timeline1D */
    void setTimeline(Timeline1D * timeline = 0);

    // --------- conversion screen/time/value --------

    Double screen2time(Double x) const;
    Double screen2value(Double y) const;
    int time2screen(Double time) const;
    int value2screen(Double val) const;

signals:

public slots:

protected:

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void clearHover_();
    void setHover_(const Timeline1D::Point&);

    /** Returns the screen rect for a given point */
    QRect handleRect_(const Timeline1D::Point&);

    // ____________ MEMBER _____________

    Timeline1D * tl_;

    // ---- config ----

    int overPaint_,
        handleRadius_;

    // ---- interaction ----

    Timeline1D::TpHash hoverHash_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TIMELINE1DVIEW_H
