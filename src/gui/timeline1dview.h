/** @file

    @brief widget for Timeline1D

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/24</p>
*/

#ifndef MOSRC_GUI_TIMELINE1DVIEW_H
#define MOSRC_GUI_TIMELINE1DVIEW_H

#include <QWidget>
#include <QSet>
#include <QVector>

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
    bool isHover_() const;
    Timeline1D::TpList::iterator hoverPoint_();

    void clearSelect_();
    void addSelect_(const Timeline1D::Point&);
    /** is anyone selected? */
    bool isSelected_() const { return !selectHashSet_.empty(); }
    void moveSelected_(Double dx, Double dy);

    /** Returns the screen rect for a given point */
    QRect handleRect_(const Timeline1D::Point&, bool hovered);

    // _________ PRIVATE TYPES _________

    enum Action_
    {
        A_NOTHING,
        A_START_DRAG_SELECTED
    };

    struct DragPoint_
    {
        Timeline1D::Point
            oldp,
            newp;
        DragPoint_() { }
        DragPoint_(const Timeline1D::Point oldp) : oldp(oldp), newp(oldp) { }
        //DragPoint_(const Timeline1D::Point oldp, const Timeline1D::Point newp) : oldp(oldp), newp(newp) { }
    };

    // ____________ MEMBER _____________

    Timeline1D * tl_;

    // ---- config ----

    int overPaint_,
        handleRadius_,
        handleRadiusHovered_;

    // ---- interaction ----

    Timeline1D::TpHash
        hoverHash_;

    QSet<Timeline1D::TpHash>
        selectHashSet_;

    QVector<DragPoint_>
        dragPoints_;

    Action_ action_;
    QPoint dragStart_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TIMELINE1DVIEW_H
