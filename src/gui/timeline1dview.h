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
#include "gui/util/viewspace.h"

namespace MO {
namespace GUI {

namespace PAINTER { class Grid; }

class Timeline1DView : public QWidget
{
    Q_OBJECT
public:

    enum Option
    {
        O_MovePointsX = 1,
        O_MovePointsY = 1<<1,
        O_AddRemovePoints = 1<<2,
        O_ChangePointType = 1<<3,

        O_MoveViewX = 1<<8,
        O_MoveViewY = 1<<9,
        O_ZoomViewX = 1<<10,
        O_ZoomViewY = 1<<11,

        O_MoveView = O_MoveViewX | O_MoveViewY,
        O_ZoomView = O_ZoomViewX | O_ZoomViewY,
        O_ChangeViewX = O_MoveViewX | O_ZoomViewX,
        O_ChangeViewY = O_MoveViewY | O_ZoomViewY,
        O_ChangeViewAll = O_ChangeViewX | O_ChangeViewY,

        O_MovePoints = O_MovePointsX | O_MovePointsY,
        O_EditAll = O_MovePoints | O_AddRemovePoints | O_ChangePointType,

        O_EnableAll = 0xffffffff
    };

    enum ClipboardType
    {
        C_NONE,
        C_WHOLE,
        C_SELECTION
    };

    // ---------- ctor -------------

    explicit Timeline1DView(MATH::Timeline1D * timeline = 0, QWidget *parent = 0);

    // ---------- getter -----------

    int options() const { return options_; }

    /** is anyone selected? */
    bool isSelected() const { return !selectHashSet_.empty(); }

    static ClipboardType isTimelineInClipboard();

    // ----------- assignment ------

    /** Assigns a new (or no) Timeline1D */
    void setTimeline(MATH::Timeline1D * timeline = 0);

    /** Sets the options for the background grid as or-wise combination of PAINTER::Grid::Option */
    void setGridOptions(int options);

    // --------- conversion screen/time/value --------

    const UTIL::ViewSpace viewSpace() const { return space_; }

    Double screen2time(Double x) const;
    Double screen2value(Double y) const;
    int time2screen(Double time) const;
    int value2screen(Double val) const;

signals:

    /** Send when viewspace was changed by user */
    void viewSpaceChanged(const UTIL::ViewSpace&);

public slots:

    void setOptions(int options);

    /** Sets a new viewspace for the timeline */
    void setViewSpace(const UTIL::ViewSpace &, bool send_signal = false);

    /** Fits the whole curve into view */
    void fitToView(bool fitX = true, bool fitY = true, int marginInPixels = 10);

    /** Fits the selected curve into view */
    void fitSelectionToView(bool fitX = true, bool fitY = true, int marginInPixels = 10);

    /** Clear any selections previously made.
        This should be called when a timeline was loaded. */
    void unselect();

    void copyAll();
    void copySelection();
    void paste();

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
        A_DRAG_SELECTED,
        A_START_SELECT_FRAME,
        A_SELECT_FRAME
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
        MATH::Timeline1D::Point
            oldp,
            newp;
        MATH::Timeline1D::TpList::iterator
            it;
        DragPoint_() { }
        DragPoint_(const MATH::Timeline1D::TpList::iterator& it) : valid(true), oldp(it->second), newp(it->second), it(it) { }
    };

    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

    void clearHover_();
    void setHover_(const MATH::Timeline1D::Point&);
    bool isHover_() const;
    MATH::Timeline1D::TpList::iterator hoverPoint_();

    void clearSelect_();
    void addSelect_(const MATH::Timeline1D::Point&, bool do_swap = false);
    void addSelect_(const QRect& rect, bool do_swap = false);
    void selectAll_();
    void selectDirection_(int dir);

    /** Returns the screen rect for a given point */
    QRect handleRect_(const MATH::Timeline1D::Point&, RectStyle_ rs);
    void updateAroundPoint_(const MATH::Timeline1D::Point&);
    void updateDerivatives_(MATH::Timeline1D::TpList::iterator it, int leftRight = 1);

    void changeScale_(int screenX, int screenY, Double factorX, Double factorY);
    void fitToView_(Double tmin, Double tmax, bool fitX, bool fitY, int marginInPixels);

    void changePointType_(MATH::Timeline1D::Point::Type t);
    void moveSelected_(Double dx, Double dy);
    void addPoint_(Double t, Double v);

    QCursor defaultCursor_() const;

    /** Limits the time to the screen when moving/zooming is disabled. */
    Double limitX_(Double time) const;
    /** Limits the value to the screen when moving/zooming is disabled. */
    Double limitY_(Double value) const;

    // ____________ MEMBER _____________

    MATH::Timeline1D * tl_;

    UTIL::ViewSpace space_;

    PAINTER::Grid * gridPainter_;

    // ---- config ----

    int options_;

    int overPaint_,
        handleRadius_,
        handleRadiusHovered_,
        handleRadiusSelected_;
    Double
        zoomChange_;
    int modifierSelectFrame_,
        modifierMultiSelect_,
        modifierMoveVert_;

    // ---- interaction ----

    MATH::Timeline1D::TpHash
        hoverHash_,
        hoverCurveHash_;

    QSet<MATH::Timeline1D::TpHash>
        selectHashSet_;

    QVector<DragPoint_>
        dragPoints_;

    UTIL::ViewSpace dragStartSpace_;

    Action_ action_;
    QPoint dragStart_,
           popupClick_;

    QRect selRect_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_TIMELINE1DVIEW_H
