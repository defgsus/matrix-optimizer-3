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
    // --------- types -------------

    struct ViewSpace
    {
        Double offsetX, offsetY,
               scaleX, scaleY;

        ViewSpace() : offsetX(0.0), offsetY(0.0), scaleX(1.0), scaleY(1.0) { }
    };

    // ---------- ctor -------------

    explicit Timeline1DView(Timeline1D * timeline = 0, QWidget *parent = 0);


    // ----------- assignment ------

    /** Assigns a new (or no) Timeline1D */
    void setTimeline(Timeline1D * timeline = 0);

    // --------- conversion screen/time/value --------

    Double screen2time(Double x) const;
    Double screen2value(Double y) const;
    int time2screen(Double time) const;
    int value2screen(Double val) const;

signals:

public slots:

    /** Fits the whole curve into view */
    void fitToView(int marginInPixels = 10);

    /** Fits the selected curve into view */
    void fitSelectionToView(int marginInPixels = 10);

protected slots:

    void slotPointContextMenu_();
    void slotEmptyContextMenu_();

protected:

    // _________ PRIVATE TYPES _________

    enum Action_
    {
        A_NOTHING,
        A_START_DRAG_SPACE,
        A_DRAG_SPACE,
        A_DRAG_SELECTED
    };

    enum RectStyle_
    {
        RS_NORMAL,
        RS_HOVER,
        RS_SELECTED,
        RS_UPDATE
    };

    struct DragPoint_
    {
        bool valid;
        Timeline1D::Point
            oldp,
            newp;
        Timeline1D::TpList::iterator
            it;
        DragPoint_() { }
        DragPoint_(const Timeline1D::TpList::iterator& it) : valid(true), oldp(it->second), newp(it->second), it(it) { }
    };

    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

    void clearHover_();
    void setHover_(const Timeline1D::Point&);
    bool isHover_() const;
    Timeline1D::TpList::iterator hoverPoint_();

    void clearSelect_();
    void addSelect_(const Timeline1D::Point&);
    void selectAll_();
    /** is anyone selected? */
    bool isSelected_() const { return !selectHashSet_.empty(); }

    /** Returns the screen rect for a given point */
    QRect handleRect_(const Timeline1D::Point&, RectStyle_ rs);

    void changeScale_(int screenX, int screenY, Double factorX, Double factorY);
    void fitToView_(Double tmin, Double tmax, int marginInPixels);

    void changePointType_(Timeline1D::Point::Type t);
    void moveSelected_(Double dx, Double dy);
    void addPoint_(Double t, Double v);

    // ____________ MEMBER _____________

    Timeline1D * tl_;

    ViewSpace space_;

    // ---- config ----

    int overPaint_,
        handleRadius_,
        handleRadiusHovered_,
        handleRadiusSelected_;
    Double
        zoomChange_;

    // ---- interaction ----

    Timeline1D::TpHash
        hoverHash_,
        hoverCurveHash_;

    QSet<Timeline1D::TpHash>
        selectHashSet_;

    QVector<DragPoint_>
        dragPoints_;

    ViewSpace dragStartSpace_;

    Action_ action_;
    QPoint dragStart_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TIMELINE1DVIEW_H
